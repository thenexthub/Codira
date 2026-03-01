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

#include <array>

#include "libarkbase/utils/utils.h"
#include "runtime/mem/malloc-proxy-allocator-inl.h"
#include "runtime/tests/allocator_test_base.h"

namespace ark::mem {

using MallocProxyNonObjectAllocator = MallocProxyAllocator<EmptyAllocConfigWithCrossingMap>;

class MallocProxyAllocatorTest : public AllocatorTest<MallocProxyNonObjectAllocator> {
public:
    NO_COPY_SEMANTIC(MallocProxyAllocatorTest);
    NO_MOVE_SEMANTIC(MallocProxyAllocatorTest);

    // NOLINTNEXTLINE(modernize-use-equals-default)
    MallocProxyAllocatorTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~MallocProxyAllocatorTest() override
    {
        // Logger::Destroy();
    }

protected:
    static constexpr size_t SIZE_ALLOC = 1_KB;

    void AddMemoryPoolToAllocator([[maybe_unused]] MallocProxyNonObjectAllocator &allocator) override {}
    void AddMemoryPoolToAllocatorProtected([[maybe_unused]] MallocProxyNonObjectAllocator &allocator) override {}
    bool AllocatedByThisAllocator([[maybe_unused]] MallocProxyNonObjectAllocator &allocator,
                                  [[maybe_unused]] void *mem) override
    {
        return false;
    }
};

TEST_F(MallocProxyAllocatorTest, SimpleTest)
{
    static constexpr size_t SIZE = 23;
    auto *memStats = new mem::MemStatsType();
    MallocProxyNonObjectAllocator allocator(memStats);
    void *a1 = allocator.Alloc(SIZE);
    allocator.Free(a1);
    delete memStats;
}

TEST_F(MallocProxyAllocatorTest, AlignedAllocFreeTest)
{
    AlignedAllocFreeTest<1, SIZE_ALLOC>();
}

TEST_F(MallocProxyAllocatorTest, AllocFreeTest)
{
    static constexpr size_t POOLS_COUNT = 1;
    AllocateFreeDifferentSizesTest<1, 4U * SIZE_ALLOC>(4U * SIZE_ALLOC, POOLS_COUNT);
}

TEST_F(MallocProxyAllocatorTest, AdapterTest)
{
    auto *memStats = new mem::MemStatsType();
    MallocProxyNonObjectAllocator allocator(memStats);
    // NOLINTBEGIN(readability-magic-numbers)
    std::array<int, 20U> arr {{12_I, 14_I, 3_I,  5_I,  43_I, 12_I, 22_I, 42_I, 89_I, 10_I,
                               89_I, 32_I, 43_I, 12_I, 43_I, 12_I, 54_I, 89_I, 27_I, 84_I}};
    // NOLINTEND(readability-magic-numbers)

    std::vector<void *> v;
    for (auto i : arr) {
        auto *mem = allocator.Alloc(i);
        v.push_back(mem);
    }
    for (auto *mem : v) {
        allocator.Free(mem);
    }
    delete memStats;
}

}  // namespace ark::mem
