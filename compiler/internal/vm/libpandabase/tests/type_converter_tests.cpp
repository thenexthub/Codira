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

#include "utils/type_converter.h"

#include <sstream>
#include <random>
#include <gtest/gtest.h>

#ifndef PANDA_NIGHTLY_TEST_ON
constexpr size_t ITERATION = 64;
#else
constexpr size_t ITERATION = 1024;
#endif

namespace panda::helpers::test {

HWTEST(TimeTest, RandomTimeConverterTest, testing::ext::TestSize.Level0)
{
    std::mt19937 gen;
    std::uniform_int_distribution<time_t> distrib_nanos_right(0, 1e3 - 1);
    std::uniform_int_distribution<time_t> distrib_nanos_left(1, 23);
    for (size_t i = 0; i < ITERATION; i++) {
        uint64_t left_part_time = distrib_nanos_left(gen);
        uint64_t right_part_time = distrib_nanos_right(gen);
        ASSERT_NE(TimeConverter(left_part_time), ValueUnit(left_part_time, "ns"));
        ASSERT_NE(TimeConverter(right_part_time), ValueUnit(right_part_time, "ns"));

        ASSERT_EQ(TimeConverter(left_part_time), ValueUnit(static_cast<double>(left_part_time), "ns"));
        ASSERT_EQ(TimeConverter(right_part_time), ValueUnit(static_cast<double>(right_part_time), "ns"));

        double expected = left_part_time + right_part_time * 1e-3;
        uint64_t nanos = left_part_time * 1e3 + right_part_time;
        ASSERT_EQ(TimeConverter(nanos), ValueUnit(expected, "us"));
        ASSERT_EQ(TimeConverter(nanos * 1e3), ValueUnit(expected, "ms"));
        ASSERT_EQ(TimeConverter(nanos * 1e6), ValueUnit(expected, "s"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60), ValueUnit(expected, "m"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60 * 60), ValueUnit(expected, "h"));
        ASSERT_EQ(TimeConverter(nanos * 1e6 * 60 * 60 * 24), ValueUnit(expected, "day"));
    }
}

HWTEST(TimeTest, RoundTimeConverterTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(TimeConverter(11'119'272), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'472), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'499), ValueUnit(11.119, "ms"));
    ASSERT_EQ(TimeConverter(11'119'500), ValueUnit(11.120, "ms"));
    ASSERT_EQ(TimeConverter(11'119'572), ValueUnit(11.120, "ms"));
    ASSERT_EQ(TimeConverter(11'119'999), ValueUnit(11.120, "ms"));
}

HWTEST(MemoryTest, RandomMemoryConverterTest, testing::ext::TestSize.Level0)
{
    std::mt19937 gen;
    std::uniform_int_distribution<uint64_t> distrib_bytes(1, 1023);
    for (size_t i = 0; i < ITERATION; i++) {
        uint64_t left_part_bytes = distrib_bytes(gen);
        uint64_t right_part_bytes = distrib_bytes(gen);
        ASSERT_NE(MemoryConverter(left_part_bytes), ValueUnit(left_part_bytes, "B"));
        ASSERT_NE(MemoryConverter(right_part_bytes), ValueUnit(right_part_bytes, "B"));

        ASSERT_EQ(MemoryConverter(left_part_bytes), ValueUnit(static_cast<double>(left_part_bytes), "B"));
        ASSERT_EQ(MemoryConverter(right_part_bytes), ValueUnit(static_cast<double>(right_part_bytes), "B"));

        double expected = left_part_bytes + right_part_bytes * 1e-3;
        uint64_t bytes = left_part_bytes * 1024 + right_part_bytes;
        ASSERT_EQ(MemoryConverter(bytes), ValueUnit(expected, "KB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 10)), ValueUnit(expected, "MB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 20)), ValueUnit(expected, "GB"));
        ASSERT_EQ(MemoryConverter(bytes * (1UL << 30)), ValueUnit(expected, "TB"));
    }
}

HWTEST(MemoryTest, RoundMemoryConverterTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(MemoryConverter(11'119'272), ValueUnit(10.604, "MB"));
    ASSERT_EQ(MemoryConverter(11'120'149), ValueUnit(10.605, "MB"));
    ASSERT_EQ(MemoryConverter(11'121'092), ValueUnit(10.606, "MB"));
}

HWTEST(TypeConverterTest, OperatorEqualTest, testing::ext::TestSize.Level0)
{
    const uint64_t number_zero = 0UL;
    const uint64_t number_one = 1UL;
    ASSERT_TRUE(ValueUnit(10.27, "MB") == ValueUnit(10.27, "MB"));
    ASSERT_FALSE(ValueUnit(10.27, "MB") == ValueUnit(10.27, "KB"));
    ASSERT_TRUE(ValueUnit(number_zero, "MB") == ValueUnit(number_zero, "MB"));
    ASSERT_FALSE(ValueUnit(number_zero, "MB") == ValueUnit(number_zero, "GB"));
    ASSERT_FALSE(ValueUnit(number_one, "MB") == ValueUnit(number_one, "KB"));
}

HWTEST(TypeConverterTest, ValueConverterTest, testing::ext::TestSize.Level0)
{
    const uint64_t number = 10UL;
    ASSERT_EQ(ValueConverter(11'119'272, ValueType::VALUE_TYPE_TIME), ValueUnit(11.119, "ms"));
    ASSERT_EQ(ValueConverter(11'119'272, ValueType::VALUE_TYPE_MEMORY), ValueUnit(10.604, "MB"));
    ASSERT_EQ(ValueConverter(number, ValueType::VALUE_TYPE_OBJECT), ValueUnit(number, ""));
}

HWTEST(TypeConverterTest, DumpTest, testing::ext::TestSize.Level0)
{
    const uint64_t number_one = 1UL;

    std::stringstream ss;
    ss << ValueUnit(10.27, "MB");
    ASSERT_EQ(ss.str(), "10.270MB");

    ss.str("");
    ss << ValueConverter(11'119'272, ValueType::VALUE_TYPE_TIME);
    ASSERT_EQ(ss.str(), "11.119ms");

    ss.str("");
    ss << ValueUnit(number_one, "KB");
    ASSERT_EQ(ss.str(), "1KB");
}
}  // namespace panda::helpers::test
