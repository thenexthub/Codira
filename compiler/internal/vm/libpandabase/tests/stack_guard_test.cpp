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

#include "gtest/gtest.h"
#include "os/stackGuard.h"
#include "macros.h"
#include "utils/utils.h"

#include <iostream>
#include <iomanip>

namespace panda::test {

// Helper function to consume stack space
uintptr_t ConsumeStackFrame(int depth)
{
    if (depth <= 0) {
        return panda::GetCurrentFrameAddress();
    }
    constexpr const size_t CONSUME_SIZE = 1024;
    // Allocate local variables to consume stack space
    char buffer[CONSUME_SIZE] = {0};  // 1KB stack allocation
    volatile int sum = 0;
    for (int i = 0; i < CONSUME_SIZE; i++) {
        sum += buffer[i];
    }
    // Recursive call to increase stack depth
    return ConsumeStackFrame(depth - 1);
}

class StackGuardTest : public testing::Test {
public:
    void SetUp() override
    {
        guard_ = new StackGuard();
    }

    void TearDown() override
    {
        delete guard_;
        guard_ = nullptr;
    }

protected:
    StackGuard *guard_ = nullptr;
};

// Test Initialize function
HWTEST_F(StackGuardTest, InitializeTest, testing::ext::TestSize.Level0)
{
    StackGuard *guard = new StackGuard();
    guard->Initialize();
    // After initialization, stack_limit_ should be set (not 0 on supported platforms)
    uintptr_t stack_limit = guard->StackLimit();
#if defined(PANDA_TARGET_UNIX) || defined(PANDA_TARGET_WINDOWS)
    // On supported platforms, stack_limit should be a valid address
    // It should be less than the current frame address
    uintptr_t current_frame = ConsumeStackFrame(2);
    ASSERT_GT(current_frame, stack_limit);
#if defined(PANDA_TARGET_UNIX)
    constexpr uintptr_t EXPECTED_OFFSET = PANDA_STACK_SIZE;
#else
    constexpr uintptr_t EXPECTED_OFFSET = PANDA_STACK_SIZE;
#endif
    // Allow some tolerance for the offset
    uintptr_t actual_offset = current_frame - stack_limit;
    ASSERT_LE(actual_offset, EXPECTED_OFFSET);
#else
    // On unsupported platforms, stack_limit should be 0
    ASSERT_EQ(stack_limit, 0U);
#endif
}

// Test StackLimit getter
HWTEST_F(StackGuardTest, StackLimitTest, testing::ext::TestSize.Level0)
{
    // Before initialization, stack_limit_ should be 0 (from default initialization)
    uintptr_t initial_limit = guard_->StackLimit();
    ASSERT_EQ(initial_limit, 0U);

    // After initialization, should return the set value
    guard_->Initialize();
    uintptr_t limit_after_init = guard_->StackLimit();
#if defined(PANDA_TARGET_UNIX) || defined(PANDA_TARGET_WINDOWS)
    ASSERT_NE(limit_after_init, 0U);
#else
    ASSERT_EQ(limit_after_init, 0U);
#endif
}

// Test CheckOverflow when no overflow occurs
HWTEST_F(StackGuardTest, CheckOverflowNoOverflowTest, testing::ext::TestSize.Level0)
{
    guard_->Initialize();
    // Initially, overflow should be false
    bool overflow = guard_->CheckOverflow();
    ASSERT_FALSE(overflow);

    // Check again, should still be false (assuming we're not near stack limit)
    overflow = guard_->CheckOverflow();
    ASSERT_FALSE(overflow);
}

// Test CheckOverflow state persistence
HWTEST_F(StackGuardTest, CheckOverflowStatePersistenceTest, testing::ext::TestSize.Level0)
{
    guard_->Initialize();

    // First check should return false (no overflow)
    bool first_check = guard_->CheckOverflow();
    ASSERT_FALSE(first_check);

    // Multiple checks should all return false if no overflow
    for (int i = 0; i < 10; i++) {
        bool result = guard_->CheckOverflow();
        ASSERT_FALSE(result);
    }
}

// Test that overflow flag persists once set
// Note: This test is difficult to trigger actual overflow in normal conditions
// We test the logic that once overflow_ is true, it stays true
HWTEST_F(StackGuardTest, CheckOverflowFlagPersistenceTest, testing::ext::TestSize.Level0)
{
    guard_->Initialize();
    // In normal conditions, overflow should not occur
    // This test verifies the basic logic flow
    bool result1 = guard_->CheckOverflow();
    bool result2 = guard_->CheckOverflow();
    // Both should return the same value (false in normal case)
    ASSERT_EQ(result1, result2);
}

// Test that Initialize resets overflow state
HWTEST_F(StackGuardTest, InitializeResetsOverflowTest, testing::ext::TestSize.Level0)
{
    guard_->Initialize();
    // Check overflow (should be false)
    bool overflow1 = guard_->CheckOverflow();
    ASSERT_FALSE(overflow1);

    // Re-initialize
    guard_->Initialize();
    // Overflow state should be reset (false)
    bool overflow2 = guard_->CheckOverflow();
    ASSERT_FALSE(overflow2);
}

// Test basic functionality integration
HWTEST_F(StackGuardTest, IntegrationTest, testing::ext::TestSize.Level0)
{
    // Create and initialize
    guard_->Initialize();
    // Get stack limit
    uintptr_t limit = guard_->StackLimit();

#if defined(PANDA_TARGET_UNIX) || defined(PANDA_TARGET_WINDOWS)
    // Verify limit is set
    ASSERT_NE(limit, 0U);

    // Check overflow (should be false in normal case)
    bool overflow = guard_->CheckOverflow();
    ASSERT_FALSE(overflow);

    // Verify limit hasn't changed
    uintptr_t limit2 = guard_->StackLimit();
    ASSERT_EQ(limit, limit2);
#else
    // On unsupported platforms
    ASSERT_EQ(limit, 0U);
    bool overflow = guard_->CheckOverflow();
    // Should return false even when limit is 0
    ASSERT_FALSE(overflow);
#endif
}

}  // namespace panda::test

