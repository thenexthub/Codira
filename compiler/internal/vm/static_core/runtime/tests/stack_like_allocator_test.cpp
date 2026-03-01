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
#include <cstdint>
#include <limits>
#include <string>

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/stack_like_allocator-inl.h"
#include "runtime/tests/allocator_test_base.h"

namespace ark::mem {
class StackLikeAllocatorTest : public AllocatorTest<StackLikeAllocator<>> {
public:
    NO_COPY_SEMANTIC(StackLikeAllocatorTest);
    NO_MOVE_SEMANTIC(StackLikeAllocatorTest);
    StackLikeAllocatorTest()
    {
        static constexpr size_t INTERNAL_MEMORY_SIZE = 256_MB;
        ark::mem::MemConfig::Initialize(0, INTERNAL_MEMORY_SIZE, 0, 0, INTERNAL_MEMORY_SIZE, 0);
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    }

    ~StackLikeAllocatorTest() override
    {
        // Logger::Destroy();
        ark::mem::MemConfig::Finalize();
    }

protected:
    void AddMemoryPoolToAllocator([[maybe_unused]] StackLikeAllocator<> &allocator) override {}
    void AddMemoryPoolToAllocatorProtected([[maybe_unused]] StackLikeAllocator<> &allocator) override {}
    bool AllocatedByThisAllocator([[maybe_unused]] StackLikeAllocator<> &allocator, [[maybe_unused]] void *mem) override
    {
        return false;
    }

    void PrintMemory(void *dst, size_t size)
    {
        std::cout << "Print at memory: ";
        auto mem = static_cast<uint8_t *>(dst);
        for (size_t i = 0; i < size; i++) {
            std::cout << *mem++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        std::cout << std::endl;
    }

    void PrintAtIndex(size_t idx, size_t size)
    {
        std::cout << "Print at index:  ";
        ASSERT(idx + size < BYTE_ARRAY_SIZE);
        auto mem = static_cast<uint8_t *>(&byteArray_[idx]);
        for (size_t i = 0; i < size; i++) {
            std::cout << *mem++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        std::cout << std::endl;
    }

    void SetUp() override
    {
        PoolManager::Initialize();
    }

    void TearDown() override
    {
        PoolManager::Finalize();
    }
};

TEST_F(StackLikeAllocatorTest, SmallAllocateTest)
{
    constexpr size_t ITERATIONS = 32;
    constexpr size_t FRAME_SIZE = 256;
    StackLikeAllocator<> alloc;
    std::array<void *, ITERATIONS + 1> array {nullptr};
    for (size_t i = 1; i <= ITERATIONS; i++) {
        array[i] = alloc.Alloc(FRAME_SIZE);
        ASSERT_NE(array[i], nullptr);
        *(static_cast<uint64_t *>(array[i])) = i;
    }
    for (size_t i = ITERATIONS; i != 0; i--) {
        ASSERT_EQ(*(static_cast<uint64_t *>(array[i])), i);
        alloc.Free(array[i]);
    }
}

template <Alignment ALIGNMENT>
void AlignmentTest(StackLikeAllocator<ALIGNMENT> &alloc)
{
    constexpr size_t MAX_SIZE = 256;
    std::array<void *, MAX_SIZE + 1> array {nullptr};
    for (size_t i = 1; i <= MAX_SIZE; i++) {
        array[i] = alloc.Alloc(i * GetAlignmentInBytes(ALIGNMENT));
        if (array[i] == nullptr) {
            break;
        }
        ASSERT_NE(array[i], nullptr);
        ASSERT_EQ(ToUintPtr(array[i]), AlignUp(ToUintPtr(array[i]), GetAlignmentInBytes(ALIGNMENT)));
        *(static_cast<uint64_t *>(array[i])) = i;
    }
    for (size_t i = MAX_SIZE; i != 0; i--) {
        if (array[i] == nullptr) {
            break;
        }
        ASSERT_EQ(*(static_cast<uint64_t *>(array[i])), i);
        alloc.Free(array[i]);
    }
}

TEST_F(StackLikeAllocatorTest, DefaultAlignmentTest)
{
    StackLikeAllocator<> alloc;
    AlignmentTest(alloc);
}

TEST_F(StackLikeAllocatorTest, NonDefaultAlignmentTest)
{
    StackLikeAllocator<Alignment::LOG_ALIGN_4> alloc4;
    AlignmentTest(alloc4);
    StackLikeAllocator<Alignment::LOG_ALIGN_5> alloc5;
    AlignmentTest(alloc5);
}

TEST_F(StackLikeAllocatorTest, CycledAllocateFreeForHugeFramesTest)
{
// Note: we only have 4GB of memory on arm32, so we need to limit the max size of the stack like allocator
// details can be found in #26461
#ifdef PANDA_TARGET_ARM32
    constexpr size_t ITERATIONS = 512;
    constexpr size_t FRAME_SIZE = 256;
    constexpr int CYCLE_COUNT = 16;
#else
    constexpr size_t ITERATIONS = 1024;
    constexpr size_t FRAME_SIZE = 512;
    constexpr int CYCLE_COUNT = 16;
#endif
    StackLikeAllocator<> alloc;
    std::vector<std::pair<void *, size_t>> vec;

    for (int j = 0; j < CYCLE_COUNT; j++) {
        for (size_t i = 1; i <= ITERATIONS; i++) {
            void *mem = alloc.Alloc(FRAME_SIZE);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << FRAME_SIZE << " bytes in " << j
                                        << " cycle, seed: " << seed_;
            vec.emplace_back(mem, SetBytesFromByteArray(mem, FRAME_SIZE));
        }
        for (size_t i = 1; i <= ITERATIONS / 2U; i++) {
            std::pair<void *, size_t> lastPair = vec.back();
            ASSERT_TRUE(CompareBytesWithByteArray(lastPair.first, FRAME_SIZE, lastPair.second))
                << "iteration: " << i << ", size: " << FRAME_SIZE << ", address: " << std::hex << lastPair.first
                << ", index in byte array: " << lastPair.second << ", seed: " << seed_;
            alloc.Free(lastPair.first);
            vec.pop_back();
        }
    }
    while (!vec.empty()) {
        std::pair<void *, size_t> lastPair = vec.back();
        ASSERT_TRUE(CompareBytesWithByteArray(lastPair.first, FRAME_SIZE, lastPair.second))
            << "vector size: " << vec.size() << ", size: " << FRAME_SIZE << ", address: " << std::hex << lastPair.first
            << ", index in byte array: " << lastPair.second << ", seed: " << seed_;
        alloc.Free(lastPair.first);
        vec.pop_back();
    }
}

TEST_F(StackLikeAllocatorTest, CheckAddrInsideAllocator)
{
    static constexpr size_t FRAME_SIZE = 256;
    static constexpr size_t ALLOCATION_SIZE = 10;
    void *invalidAddr = std::malloc(ALLOCATION_SIZE);  // NOLINT(cppcoreguidelines-no-malloc)

    StackLikeAllocator<> alloc;
    ASSERT_FALSE(alloc.Contains(invalidAddr));  // NOLINT(clang-analyzer-unix.Malloc)

    void *addr1Inside = alloc.Alloc(FRAME_SIZE);
    ASSERT_TRUE(alloc.Contains(addr1Inside));
    ASSERT_FALSE(alloc.Contains(invalidAddr));  // NOLINT(clang-analyzer-unix.Malloc)

    alloc.Free(addr1Inside);
    ASSERT_FALSE(alloc.Contains(addr1Inside));
    ASSERT_FALSE(alloc.Contains(invalidAddr));

    addr1Inside = alloc.Alloc(FRAME_SIZE);

    auto *addr2Inside = alloc.Alloc(FRAME_SIZE * 2);
    ASSERT_TRUE(alloc.Contains(addr1Inside));
    ASSERT_TRUE(alloc.Contains(addr2Inside));
    ASSERT_FALSE(alloc.Contains(invalidAddr));
    free(invalidAddr);  // NOLINT(cppcoreguidelines-no-malloc)
}
}  // namespace ark::mem
