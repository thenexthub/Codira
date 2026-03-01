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

#include <gtest/gtest.h>

#include "utils/ring_buffer.h"

namespace panda::test {
static constexpr size_t DEFAULT_BUFFER_SIZE = 10U;

TEST(RingBufferTest, InitializeTest)
{
    constexpr RingBuffer<int, DEFAULT_BUFFER_SIZE> buffer;

    ASSERT_EQ(buffer.size(), 0U);
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buffer.begin(), buffer.end());
    ASSERT_EQ(buffer.cbegin(), buffer.cend());
    ASSERT_TRUE(buffer.empty());
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_FALSE(buffer.full());
}

TEST(RingBufferTest, PushToBufferTest)
{
    RingBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;

    for (size_t i = 0; i < DEFAULT_BUFFER_SIZE; ++i) {
        buffer.push_back(i);
        ASSERT_EQ(buffer.size(), i + 1);
        ASSERT_EQ(buffer.front(), 0U);
        ASSERT_EQ(buffer.back(), i);
    }

    ASSERT_EQ(buffer.size(), DEFAULT_BUFFER_SIZE);
    ASSERT_NE(buffer.begin(), buffer.end());
    ASSERT_FALSE(buffer.empty());
    ASSERT_TRUE(buffer.full());

    size_t i = 0;
    for (const auto &element : buffer) {
        ASSERT_EQ(element, i++);
    }

    // Add new element when buffer is full
    buffer.emplace_back(i);

    ASSERT_EQ(*buffer.begin(), 1U);
    ASSERT_EQ(buffer.front(), 1U);
    ASSERT_EQ(buffer.back(), i);
    ASSERT_EQ(buffer.size(), DEFAULT_BUFFER_SIZE);
    ASSERT_NE(buffer.begin(), buffer.end());
    ASSERT_FALSE(buffer.empty());
    ASSERT_TRUE(buffer.full());

    i = 1;
    for (const auto &element : buffer) {
        ASSERT_EQ(element, i++);
    }
    for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
        ASSERT_EQ(*it, --i);
    }

    auto [mi, ma] = std::minmax_element(buffer.begin(), buffer.end());
    ASSERT_EQ(*mi, 1U);
    ASSERT_EQ(*ma, DEFAULT_BUFFER_SIZE);

    buffer.clear();
    ASSERT_EQ(buffer.size(), 0U);
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_TRUE(buffer.empty());
    ASSERT_FALSE(buffer.full());
    ASSERT_EQ(buffer.begin(), buffer.end());
}

TEST(RingBufferTest, PushPopToBufferTest)
{
    RingBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;

    for (size_t i = 0; i < DEFAULT_BUFFER_SIZE; ++i) {
        buffer.push_back(i);
        ASSERT_EQ(buffer.size(), i + 1);
        ASSERT_EQ(buffer.front(), 0U);
        ASSERT_EQ(buffer.back(), i);
    }
    ASSERT_EQ(buffer.size(), DEFAULT_BUFFER_SIZE);
    ASSERT_TRUE(buffer.full());

    static_assert(DEFAULT_BUFFER_SIZE >= 3U, "Need 3 elements for tests");
    size_t i = DEFAULT_BUFFER_SIZE - 1U;
    buffer.pop_back();
    ASSERT_EQ(buffer.size(), DEFAULT_BUFFER_SIZE - 1U);
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buffer.front(), 0U);
    ASSERT_EQ(buffer.back(), i - 1U);
    ASSERT_NE(buffer.begin(), buffer.end());
    ASSERT_FALSE(buffer.empty());
    ASSERT_FALSE(buffer.full());

    buffer.pop_front();
    ASSERT_EQ(buffer.size(), DEFAULT_BUFFER_SIZE - 2U);
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buffer.front(), 1U);
    ASSERT_EQ(buffer.back(), i - 1U);
    ASSERT_NE(buffer.begin(), buffer.end());
    ASSERT_FALSE(buffer.empty());
    ASSERT_FALSE(buffer.full());

    i = 1U;
    for (const auto &element : buffer) {
        ASSERT_EQ(element, i++);
    }
    ASSERT_EQ(i, DEFAULT_BUFFER_SIZE - 1U);

    buffer.clear();
    ASSERT_EQ(buffer.size(), 0U);
    ASSERT_EQ(buffer.capacity(), DEFAULT_BUFFER_SIZE);
    ASSERT_TRUE(buffer.empty());
    ASSERT_FALSE(buffer.full());
    ASSERT_EQ(buffer.begin(), buffer.end());
}
}  // namespace panda::test
