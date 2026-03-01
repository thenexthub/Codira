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

#include "libarkbase/utils/type_converter.h"

#include <sstream>
#include <random>
#include <gtest/gtest.h>

#ifndef PANDA_NIGHTLY_TEST_ON
constexpr size_t ITERATION = 64;
#else
constexpr size_t ITERATION = 1024;
#endif

namespace ark::helpers::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST(TimeTest, RandomTimeConverterTest)
{
    // NOLINTNEXTLINE(cert-msc51-cpp)
    std::mt19937 gen;
    std::uniform_int_distribution<time_t> distribNanosRight(0U, 1e3 - 1L);
    std::uniform_int_distribution<time_t> distribNanosLeft(1U, 23U);
    for (size_t i = 0; i < ITERATION; i++) {
        uint64_t leftPartTime = distribNanosLeft(gen);
        uint64_t rightPartTime = distribNanosRight(gen);
        ASSERT_NE(TimeConverter(leftPartTime), ValueUnit(leftPartTime, "ns"));
        ASSERT_NE(TimeConverter(rightPartTime), ValueUnit(rightPartTime, "ns"));

        ASSERT_EQ(TimeConverter(leftPartTime), ValueUnit(static_cast<double>(leftPartTime), "ns"));
        ASSERT_EQ(TimeConverter(rightPartTime), ValueUnit(static_cast<double>(rightPartTime), "ns"));

        double expected = leftPartTime + rightPartTime * 1e-3;
        uint64_t nanos = leftPartTime * 1e3 + rightPartTime;
        ASSERT_EQ(TimeConverter(nanos), ValueUnit(expected, "us"));
        ASSERT_EQ(TimeConverter(nanos * 1e3), ValueUnit(expected, "ms"));
        ASSERT_EQ(TimeConverter(nanos * 1e6), ValueUnit(expected, "s"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60), ValueUnit(expected, "m"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60 * 60), ValueUnit(expected, "h"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60 * 60 * 24), ValueUnit(expected, "day"));
    }
}

TEST(TimeTest, RoundTimeConverterTest)
{
    ASSERT_EQ(TimeConverter(11'119'272), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'472), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'499), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'500), ValueUnit(11.120, "ms"));
    ASSERT_EQ(TimeConverter(11'119'572), ValueUnit(11.120, "ms"));
    ASSERT_EQ(TimeConverter(11'119'999), ValueUnit(11.120, "ms"));
}

TEST(MemoryTest, RandomMemoryConverterTest)
{
    // NOLINTNEXTLINE(cert-msc51-cpp)
    std::mt19937 gen;
    std::uniform_int_distribution<uint64_t> distribBytes(1U, 1023U);
    for (size_t i = 0; i < ITERATION; i++) {
        uint64_t leftPartBytes = distribBytes(gen);
        uint64_t rightPartBytes = distribBytes(gen);
        ASSERT_NE(MemoryConverter(leftPartBytes), ValueUnit(leftPartBytes, "B"));
        ASSERT_NE(MemoryConverter(rightPartBytes), ValueUnit(rightPartBytes, "B"));

        ASSERT_EQ(MemoryConverter(leftPartBytes), ValueUnit(static_cast<double>(leftPartBytes), "B"));
        ASSERT_EQ(MemoryConverter(rightPartBytes), ValueUnit(static_cast<double>(rightPartBytes), "B"));

        double expected = leftPartBytes + rightPartBytes * 1e-3;
        uint64_t bytes = leftPartBytes * 1024U + rightPartBytes;
        ASSERT_EQ(MemoryConverter(bytes), ValueUnit(expected, "KB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 10U)), ValueUnit(expected, "MB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 20U)), ValueUnit(expected, "GB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 30U)), ValueUnit(expected, "TB"));
    }
}

// NOLINTEND(readability-magic-numbers)

TEST(MemoryTest, RoundMemoryConverterTest)
{
    ASSERT_EQ(MemoryConverter(11'119'272), ValueUnit(10.604, "MB"));
    ASSERT_EQ(MemoryConverter(11'120'149), ValueUnit(10.605, "MB"));
    ASSERT_EQ(MemoryConverter(11'121'092), ValueUnit(10.606, "MB"));
}

}  // namespace ark::helpers::test
