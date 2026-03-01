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

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/tests/allocator_test_base.h"
#include "runtime/mem/internal_allocator-inl.h"

#include <gtest/gtest.h>

namespace ark::mem::test {

class InternalAllocatorTest : public testing::Test {
public:
    InternalAllocatorTest()
    {
        ark::mem::MemConfig::Initialize(0, MEMORY_POOL_SIZE, 0, 0, 0, 0);
        PoolManager::Initialize();
        memStats_ = new mem::MemStatsType();
        allocator_ = new InternalAllocatorT<InternalAllocatorConfig::PANDA_ALLOCATORS>(memStats_);
    }

    ~InternalAllocatorTest() override
    {
        delete static_cast<Allocator *>(allocator_);
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        delete memStats_;
    }

    NO_COPY_SEMANTIC(InternalAllocatorTest);
    NO_MOVE_SEMANTIC(InternalAllocatorTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    InternalAllocatorPtr allocator_;

    static constexpr size_t MEMORY_POOL_SIZE = 16_MB;

    void InfinitiveAllocate(size_t allocSize)
    {
        void *mem = nullptr;
        do {
            mem = allocator_->Alloc(allocSize);
        } while (mem != nullptr);
    }

    // Check that we don't have OOM and there is free space for mem pools
    bool CheckFreeSpaceForPools()
    {
        size_t currentSpaceSize = PoolManager::GetMmapMemPool()
                                      ->nonObjectSpacesCurrentSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_INTERNAL)];
        size_t maxSpaceSize =
            PoolManager::GetMmapMemPool()->nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_INTERNAL)];
        ASSERT(currentSpaceSize <= maxSpaceSize);
        return (maxSpaceSize - currentSpaceSize) >= InternalAllocator<>::RunSlotsAllocatorT::GetMinPoolSize();
    }

private:
    mem::MemStatsType *memStats_;
};

TEST_F(InternalAllocatorTest, AvoidInfiniteLoopTest)
{
    // Regular object sizes
    InfinitiveAllocate(RunSlots<>::MaxSlotSize());
    // Large object sizes
    InfinitiveAllocate(FreeListAllocator<EmptyMemoryConfig>::GetMaxSize());
    // Humongous object sizes
    InfinitiveAllocate(FreeListAllocator<EmptyMemoryConfig>::GetMaxSize() + 1);
}

struct A {
    NO_COPY_SEMANTIC(A);
    NO_MOVE_SEMANTIC(A);

    static size_t count_;
    A()
    {
        value = ++count_;
    }
    ~A()
    {
        --count_;
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    uint8_t value;
};

size_t A::count_ = 0;

TEST_F(InternalAllocatorTest, NewDeleteArray)
{
    constexpr size_t COUNT = 5;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto arr = allocator_->New<A[]>(COUNT);
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(ToUintPtr(arr) % DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES, 0);
    ASSERT_EQ(A::count_, COUNT);
    for (uint8_t i = 1; i <= COUNT; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT_EQ(arr[i - 1].value, i);
    }
    allocator_->DeleteArray(arr);
    ASSERT_EQ(A::count_, 0);
}

TEST_F(InternalAllocatorTest, ZeroSizeTest)
{
    ASSERT(allocator_->Alloc(0) == nullptr);
    // Check that zero-size allocation did not result in infinite pool allocations
    ASSERT(CheckFreeSpaceForPools());

    // Checks on correct allocations of different size
    // Regular object size
    void *mem = allocator_->Alloc(RunSlots<>::MaxSlotSize());
    ASSERT(mem != nullptr);
    allocator_->Free(mem);

    // Large object size
    mem = allocator_->Alloc(FreeListAllocator<EmptyMemoryConfig>::GetMaxSize());
    ASSERT(mem != nullptr);
    allocator_->Free(mem);

    // Humongous object size
    mem = allocator_->Alloc(FreeListAllocator<EmptyMemoryConfig>::GetMaxSize() + 1);
    ASSERT(mem != nullptr);
    allocator_->Free(mem);
}

TEST_F(InternalAllocatorTest, AllocAlignmentTest)
{
    constexpr size_t ALIGNMENT = DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES * 2U;
    constexpr size_t N = RunSlots<>::MaxSlotSize() + DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES;

    struct alignas(ALIGNMENT) S {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        uint8_t a[N];
    };

    auto isAligned = [](void *ptr) { return IsAligned(reinterpret_cast<uintptr_t>(ptr), ALIGNMENT); };

    auto *ptr = allocator_->Alloc(N);
    if (!isAligned(ptr)) {
        allocator_->Free(ptr);
        ptr = nullptr;
    }

    {
        auto *p = allocator_->AllocArray<S>(1);
        ASSERT_TRUE(isAligned(p));
        allocator_->Free(p);
    }

    {
        auto *p = allocator_->New<S>();
        ASSERT_TRUE(isAligned(p));
        allocator_->Delete(p);
    }

    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto *p = allocator_->New<S[]>(1);
        ASSERT_TRUE(isAligned(p));
        allocator_->DeleteArray(p);
    }

    if (ptr != nullptr) {
        allocator_->Free(ptr);
    }
}

TEST_F(InternalAllocatorTest, AllocLocalAlignmentTest)
{
    constexpr size_t ALIGNMENT = DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES * 2U;
    constexpr size_t N = RunSlots<>::MaxSlotSize() + DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES;

    struct alignas(ALIGNMENT) S {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        uint8_t a[N];
    };

    auto isAligned = [](void *ptr) { return IsAligned(reinterpret_cast<uintptr_t>(ptr), ALIGNMENT); };

    auto *ptr = allocator_->AllocLocal(N);
    if (!isAligned(ptr)) {
        allocator_->Free(ptr);
        ptr = nullptr;
    }

    {
        auto *p = allocator_->AllocArrayLocal<S>(1);
        ASSERT_TRUE(isAligned(p));
        allocator_->Free(p);
    }

    {
        auto *p = allocator_->AllocArrayLocal<S>(1);
        ASSERT_TRUE(isAligned(p));
        allocator_->Free(p);
    }

    {
        auto *p = allocator_->NewLocal<S>();
        ASSERT_TRUE(isAligned(p));
        allocator_->Delete(p);
    }

    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto *p = allocator_->NewLocal<S[]>(1);
        ASSERT_TRUE(isAligned(p));
        allocator_->DeleteArray(p);
    }

    if (ptr != nullptr) {
        allocator_->Free(ptr);
    }
}

TEST_F(InternalAllocatorTest, MoveContainerTest)
{
    using TestValueType = int;
    constexpr auto MAGIC_VALUE = TestValueType {};
    AllocatorAdapter<TestValueType> adapter = allocator_->Adapter();
    using TestVector = std::vector<TestValueType, decltype(adapter)>;
    TestVector vector1(adapter);
    TestVector vector2(adapter);
    // Swap
    auto vector3 = std::move(vector2);
    vector2 = std::move(vector1);
    vector1 = std::move(vector3);
    vector2.emplace_back(MAGIC_VALUE);
    ASSERT_EQ(vector2.back(), MAGIC_VALUE);
    ASSERT_EQ(vector2.get_allocator(), adapter);

    using TestDeque = std::deque<TestValueType, decltype(adapter)>;
    TestDeque *dequeTmp = allocator_->New<TestDeque>(allocator_->Adapter());
    *dequeTmp = TestDeque(allocator_->Adapter());
    dequeTmp->push_back(MAGIC_VALUE);
    ASSERT_EQ(dequeTmp->get_allocator(), allocator_->Adapter());
    ASSERT_EQ(dequeTmp->back(), MAGIC_VALUE);
    allocator_->Delete(dequeTmp);
}

}  // namespace ark::mem::test
