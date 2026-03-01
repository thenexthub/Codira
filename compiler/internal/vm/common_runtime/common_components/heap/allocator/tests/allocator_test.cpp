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

#include "common_components/heap/allocator/allocator.h"
#include "common_components/tests/test_helper.h"
#include <cstdint>
#include <gtest/gtest.h>

using namespace common;

namespace common::test {

class TestAllocator : public Allocator {
public:
    bool GetIsAsyncAllocationEnable() const
    {
        return isAsyncAllocationEnable_;
    }
    HeapAddress Allocate(size_t size, AllocType allocType) override { return 0; }
    HeapAddress AllocateNoGC(size_t size, AllocType allocType) override { return 0; }
    bool ForEachObject(const std::function<void(BaseObject*)>&, bool safe) const override { return true; }
    size_t ReclaimGarbageMemory(bool releaseAll) override { return 0; }
    size_t LargeObjectSize() const override { return 0; }
    size_t GetAllocatedBytes() const override { return 0; }
    void Init(const RuntimeParam& param) override {}
    size_t GetMaxCapacity() const override { return 0; }
    size_t GetCurrentCapacity() const override { return 0; }
    size_t GetUsedPageSize() const override { return 0; }
    HeapAddress GetSpaceStartAddress() const override { return 0; }
    HeapAddress GetSpaceEndAddress() const override { return 0; }
#ifndef NDEBUG
    bool IsHeapObject(HeapAddress) const override { return false; }
#endif
    void FeedHungryBuffers() override {}
    size_t GetSurvivedSize() const override { return 0; }
};
class AllocatorTest : public common::test::BaseTestWithScope {
};

HWTEST_F_L0(AllocatorTest, EnvNotSet)
{
    unsetenv("arkEnableAsyncAllocation");

    TestAllocator allocator;
    bool result = allocator.GetIsAsyncAllocationEnable();
#if defined(PANDA_TARGET_OHOS)
    EXPECT_TRUE(result);
#else
    EXPECT_FALSE(result);
#endif
}

HWTEST_F_L0(AllocatorTest, InvalidLength)
{
    setenv("arkEnableAsyncAllocation", "12", 1);

    TestAllocator allocator;
    bool result = allocator.GetIsAsyncAllocationEnable();

#if defined(PANDA_TARGET_OHOS)
    EXPECT_TRUE(result);
#else
    EXPECT_FALSE(result);
#endif
}

HWTEST_F_L0(AllocatorTest, InitAsyncAllocation_ValidLength)
{
    setenv("arkEnableAsyncAllocation", "1", 1);
    TestAllocator allocator;
    bool result = allocator.GetIsAsyncAllocationEnable();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(AllocatorTest, InitAsyncAllocation_Zero)
{
    setenv("arkEnableAsyncAllocation", "0", 1);
    TestAllocator allocator;
    bool result = allocator.GetIsAsyncAllocationEnable();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(AllocatorTest, InitAsyncAllocation_InvalidChar)
{
    setenv("arkEnableAsyncAllocation", "a", 1);
    TestAllocator allocator;
    bool result = allocator.GetIsAsyncAllocationEnable();
    EXPECT_TRUE(result);
}
}
