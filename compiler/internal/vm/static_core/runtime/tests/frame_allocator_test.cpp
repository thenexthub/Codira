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
#include "runtime/mem/frame_allocator-inl.h"
#include "runtime/tests/allocator_test_base.h"

namespace ark::mem {

class FrameAllocatorTest : public AllocatorTest<FrameAllocator<>> {
public:
    FrameAllocatorTest()
    {
        static constexpr size_t INTERNAL_SIZE = 256_MB;
        static constexpr size_t FRAME_SIZE = 256_MB;
        ark::mem::MemConfig::Initialize(0, INTERNAL_SIZE, 0, 0, FRAME_SIZE, 0);
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    }

    ~FrameAllocatorTest() override
    {
        // Logger::Destroy();
        ark::mem::MemConfig::Finalize();
    }

    NO_MOVE_SEMANTIC(FrameAllocatorTest);
    NO_COPY_SEMANTIC(FrameAllocatorTest);

    void SmallAllocateTest(bool useMalloc) const
    {
        constexpr size_t ITERATIONS = 32;
        constexpr size_t FRAME_SIZE = 256;
        FrameAllocator<> alloc(useMalloc);
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

    void CornerAllocationSizeTest(bool useMalloc) const
    {
        constexpr size_t FIRST_FRAME_SIZE = FrameAllocator<>::FIRST_ARENA_SIZE;
        constexpr size_t SECOND_FRAME_SIZE = FrameAllocator<>::ARENA_SIZE_GREW_LEVEL;
        constexpr size_t THIRD_FRAME_SIZE = FIRST_FRAME_SIZE + SECOND_FRAME_SIZE;
        FrameAllocator<> alloc1(useMalloc);
        FrameAllocator<> alloc2(useMalloc);
        FrameAllocator<> alloc3(useMalloc);
        {
            void *mem = alloc1.Alloc(FIRST_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
            alloc1.Free(mem);
            DeallocateLastArena(&alloc1);
            mem = alloc1.Alloc(SECOND_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
        }
        {
            void *mem = alloc2.Alloc(SECOND_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
            alloc2.Free(mem);
            DeallocateLastArena(&alloc2);
            mem = alloc2.Alloc(THIRD_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
        }
        {
            void *mem = alloc3.Alloc(THIRD_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
            alloc3.Free(mem);
            DeallocateLastArena(&alloc3);
            mem = alloc3.Alloc(THIRD_FRAME_SIZE);
            ASSERT_NE(mem, nullptr);
        }
        ASSERT_NE(alloc1.Alloc(FIRST_FRAME_SIZE), nullptr);
        ASSERT_NE(alloc2.Alloc(SECOND_FRAME_SIZE), nullptr);
        ASSERT_NE(alloc3.Alloc(THIRD_FRAME_SIZE), nullptr);
    }

    template <Alignment ALIGNMENT>
    void AlignmentTest(bool useMalloc) const
    {
        FrameAllocator<ALIGNMENT> alloc(useMalloc);
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

    void CycledAllocateFreeForHugeFramesTest(bool useMalloc)
    {
        constexpr size_t ITERATIONS = 1024;
        constexpr size_t FRAME_SIZE = 512;
        constexpr int CYCLE_COUNT = 16;

        FrameAllocator<> alloc(useMalloc);
        std::vector<std::pair<void *, size_t>> vec;

        for (int j = 0; j < CYCLE_COUNT; j++) {
            for (size_t i = 1; i <= ITERATIONS; i++) {
                void *mem = alloc.Alloc(FRAME_SIZE);
                ASSERT_TRUE(mem != nullptr)
                    << "Didn't allocate " << FRAME_SIZE << " bytes in " << j << " cycle, seed: " << seed_;
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
                << "vector size: " << vec.size() << ", size: " << FRAME_SIZE << ", address: " << std::hex
                << lastPair.first << ", index in byte array: " << lastPair.second << ", seed: " << seed_;
            alloc.Free(lastPair.first);
            vec.pop_back();
        }
    }

    void ValidateArenaGrownPolicy(bool useMalloc) const
    {
        constexpr size_t ITERATIONS = 16;
        FrameAllocator<> alloc(useMalloc);
        size_t lastAllocArenaSize = 0;
        for (size_t i = 0; i < ITERATIONS; i++) {
            size_t newArenaSize = AllocNewArena(&alloc);
            ASSERT_EQ(newArenaSize > lastAllocArenaSize, true);
            lastAllocArenaSize = newArenaSize;
        }

        for (size_t i = 0; i < ITERATIONS; i++) {
            DeallocateLastArena(&alloc);
        }

        size_t newArenaSize = AllocNewArena(&alloc);
        ASSERT_EQ(newArenaSize == lastAllocArenaSize, true);
    }

    void CheckAddrInsideAllocator(bool useMalloc) const
    {
        constexpr size_t ITERATIONS = 16;
        static constexpr size_t FRAME_SIZE = 256;
        static constexpr size_t MALLOC_ALLOC_SIZE = 10;
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        void *invalidAddr = std::malloc(MALLOC_ALLOC_SIZE);

        FrameAllocator<> alloc(useMalloc);
        // NOLINTNEXTLINE(clang-analyzer-unix.Malloc)
        ASSERT_FALSE(alloc.Contains(invalidAddr));
        for (size_t i = 0; i < ITERATIONS; i++) {
            AllocNewArena(&alloc);
        }
        void *addr1Inside = alloc.Alloc(FRAME_SIZE);
        ASSERT_TRUE(alloc.Contains(addr1Inside));
        ASSERT_FALSE(alloc.Contains(invalidAddr));

        alloc.Free(addr1Inside);
        ASSERT_FALSE(alloc.Contains(addr1Inside));
        ASSERT_FALSE(alloc.Contains(invalidAddr));

        addr1Inside = alloc.Alloc(FRAME_SIZE);
        for (size_t i = 0; i < ITERATIONS; i++) {
            AllocNewArena(&alloc);
        }
        auto *addr2Inside = alloc.Alloc(FRAME_SIZE * 2);
        ASSERT_TRUE(alloc.Contains(addr1Inside));
        ASSERT_TRUE(alloc.Contains(addr2Inside));
        ASSERT_FALSE(alloc.Contains(invalidAddr));
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        free(invalidAddr);
    }

protected:
    void AddMemoryPoolToAllocator([[maybe_unused]] FrameAllocator<> &allocator) override {}
    void AddMemoryPoolToAllocatorProtected([[maybe_unused]] FrameAllocator<> &allocator) override {}
    bool AllocatedByThisAllocator([[maybe_unused]] FrameAllocator<> &allocator, [[maybe_unused]] void *mem) override
    {
        return false;
    }

    void PrintMemory(void *dst, size_t size)
    {
        std::cout << "Print at memory: ";
        Span<uint8_t> memory(static_cast<uint8_t *>(dst), size);
        for (size_t i = 0; i < size; i++) {
            std::cout << memory[i];
        }
        std::cout << std::endl;
    }

    void PrintAtIndex(size_t idx, size_t size)
    {
        std::cout << "Print at index:  ";
        ASSERT(idx + size < BYTE_ARRAY_SIZE);
        Span<uint8_t> memory(static_cast<uint8_t *>(&byteArray_[idx]), size);
        for (size_t i = 0; i < size; i++) {
            std::cout << memory[i];
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

    size_t AllocNewArena(FrameAllocator<> *alloc) const
    {
        bool isAllocated = alloc->TryAllocateNewArena();
        ASSERT(isAllocated);
        return isAllocated ? alloc->biggestArenaSize_ : 0;
    }

    void DeallocateLastArena(FrameAllocator<> *alloc) const
    {
        alloc->FreeLastArena();
    }
};

TEST_F(FrameAllocatorTest, SmallAllocateTest)
{
    SmallAllocateTest(false);
    SmallAllocateTest(true);
}

TEST_F(FrameAllocatorTest, CornerAllocationSizeTest)
{
    CornerAllocationSizeTest(false);
    CornerAllocationSizeTest(true);
}

TEST_F(FrameAllocatorTest, DefaultAlignmentTest)
{
    AlignmentTest<DEFAULT_FRAME_ALIGNMENT>(false);
    AlignmentTest<DEFAULT_FRAME_ALIGNMENT>(true);
}

TEST_F(FrameAllocatorTest, NonDefaultAlignmentTest)
{
    AlignmentTest<Alignment::LOG_ALIGN_4>(false);
    AlignmentTest<Alignment::LOG_ALIGN_4>(true);
    AlignmentTest<Alignment::LOG_ALIGN_5>(false);
    AlignmentTest<Alignment::LOG_ALIGN_5>(true);
}

TEST_F(FrameAllocatorTest, CycledAllocateFreeForHugeFramesTest)
{
    CycledAllocateFreeForHugeFramesTest(false);
    CycledAllocateFreeForHugeFramesTest(true);
}

TEST_F(FrameAllocatorTest, ValidateArenaGrownPolicy)
{
    ValidateArenaGrownPolicy(false);
    ValidateArenaGrownPolicy(true);
}

TEST_F(FrameAllocatorTest, CheckAddrInsideAllocator)
{
    CheckAddrInsideAllocator(false);
    CheckAddrInsideAllocator(true);
}

}  // namespace ark::mem
