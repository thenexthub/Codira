/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/coroutines/native_stack_allocator/native_stack_allocator.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "gtest/gtest.h"

namespace ark::mem::test {

class NativeStackAllocatorTest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        Runtime::Create(options);
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }
};

TEST_F(NativeStackAllocatorTest, SequentialAllocationAndDeallocation)
{
    NativeStackAllocator sm;
    static constexpr size_t SIZE_OF_POOL = 1024U;
    sm.Initialize(SIZE_OF_POOL);
    {
        uint8_t *firstStack = sm.AcquireStack();
        uint8_t *secondStack = sm.AcquireStack();
        ASSERT_EQ(secondStack, firstStack + SIZE_OF_POOL);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        sm.ReleaseStack(firstStack);
        uint8_t *thirdStack = sm.AcquireStack();
        ASSERT_EQ(firstStack, thirdStack);

        sm.ReleaseStack(secondStack);
        sm.ReleaseStack(thirdStack);
    }
    {
        std::array<uint8_t *, NativeStackAllocator::STACK_COUNT_IN_POOL> stackArray {};
        for (size_t i = 0; i < NativeStackAllocator::STACK_COUNT_IN_POOL; i++) {
            stackArray[i] = sm.AcquireStack();
        }
        for (size_t i = 1; i < NativeStackAllocator::STACK_COUNT_IN_POOL; i++) {
            ASSERT_EQ(stackArray[i], stackArray[i - 1] + SIZE_OF_POOL);
        }
        auto *nextStack = sm.AcquireStack();
        sm.ReleaseStack(nextStack);
        for (size_t i = 0; i < NativeStackAllocator::STACK_COUNT_IN_POOL; i++) {
            sm.ReleaseStack(stackArray[i]);
        }
    }
    sm.Finalize();
}

}  // namespace ark::mem::test
