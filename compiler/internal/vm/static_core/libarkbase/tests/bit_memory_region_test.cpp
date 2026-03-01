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

#include "libarkbase/utils/bit_memory_region-inl.h"

#include <gtest/gtest.h>
#include <array>

namespace ark::test {

static void CompareData(const uint8_t *data, size_t offset, size_t length, uint32_t value, uint8_t fillValue)
{
    for (size_t i = 0; i < length; i++) {
        uint8_t expected = (offset <= i && i < offset + length) ? value >> (i - offset) : fillValue;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t actual = data[i / BITS_PER_BYTE] >> (i % BITS_PER_BYTE);
        ASSERT_EQ(expected & 1U, actual & 1U);
    }
}

// NOLINTBEGIN(readability-magic-numbers)
TEST(BitMemoryRegion, TestBitAccess)
{
    std::array<uint8_t, 16U> data {};
    static constexpr std::array<uint8_t, 2U> FILL_DATA = {0x00U, 0xffU};
    static constexpr std::array<bool, 2U> VALUE_DATA = {false, true};
    static constexpr size_t MAX_BITS_COUNT = (data.size() - sizeof(uint32_t)) * BITS_PER_BYTE;

    for (size_t offset = 0; offset < MAX_BITS_COUNT; offset++) {
        for (auto fillValue : FILL_DATA) {
            for (auto value : VALUE_DATA) {
                std::fill(data.begin(), data.end(), fillValue);
                BitMemoryRegion region1(data.data(), offset, 1U);
                region1.Write(value, 0U);
                ASSERT_EQ(region1.Read(0U), value);
                CompareData(data.data(), offset, 1U, static_cast<uint32_t>(value), fillValue);
                std::fill(data.begin(), data.end(), fillValue);
                BitMemoryRegion region2(data.data(), data.size() * BITS_PER_BYTE);
                region2.Write(value, offset);
                ASSERT_EQ(region2.Read(offset), value);
                CompareData(data.data(), offset, 1U, static_cast<uint32_t>(value), fillValue);
            }
        }
    }
}

TEST(BitMemoryRegion, TestBitsAccess)
{
    std::array<uint8_t, 16U> data {};
    static constexpr std::array<uint8_t, 2U> FILL_DATA = {0x00U, 0xffU};
    static constexpr size_t MAX_BITS_COUNT = (data.size() - sizeof(uint32_t)) * BITS_PER_BYTE;

    for (size_t offset = 0; offset < MAX_BITS_COUNT; offset++) {
        uint32_t mask = 0;
        for (size_t length = 0; length < BITS_PER_UINT32; length++) {
            const uint32_t value = 0xBADDCAFEU & mask;
            for (auto fillValue : FILL_DATA) {
                std::fill(data.begin(), data.end(), fillValue);
                BitMemoryRegion region1(data.data(), offset, length);
                region1.Write(value, 0U, length);
                ASSERT_EQ(region1.Read(0U, length), value);
                CompareData(data.data(), offset, length, value, fillValue);
                std::fill(data.begin(), data.end(), fillValue);
                BitMemoryRegion region2(data.data(), data.size() * BITS_PER_BYTE);
                region2.Write(value, offset, length);
                ASSERT_EQ(region2.Read(offset, length), value);
                CompareData(data.data(), offset, length, value, fillValue);
            }
            mask = (mask << 1U) | 1U;
        }
    }
}

TEST(BitMemoryRegion, DumpingPart1)
{
    std::array<uint64_t, 4U> data {};
    std::stringstream ss;
    auto clear = [&data, &ss]() {
        data.fill(0U);
        ss.str(std::string());
    };

    {
        clear();
        BitMemoryRegion region(data.data(), 0U, data.size() * BITS_PER_UINT64);
        ss << region;
        ASSERT_EQ(ss.str(), "0x0");
    }

    {
        clear();
        data[0U] = 0x5;
        BitMemoryRegion region(data.data(), 0U, 130U);
        ss << region;
        ASSERT_EQ(ss.str(), "0x5");
    }

    {
        clear();
        data[0U] = 0x1;
        data[1U] = 0x2;
        BitMemoryRegion region(data.data(), 1U, 65U);
        ss << region;
        ASSERT_EQ(ss.str(), "0x10000000000000000");
    }

    {
        clear();
        data[0U] = 0x1;
        data[1U] = 0x500;
        BitMemoryRegion region(data.data(), 0U, 129U);
        ss << region;
        ASSERT_EQ(ss.str(), "0x5000000000000000001");
    }
}

TEST(BitMemoryRegion, DumpingPart2)
{
    std::array<uint64_t, 4U> data {};
    std::stringstream ss;
    auto clear = [&data, &ss]() {
        data.fill(0U);
        ss.str(std::string());
    };

    {
        clear();
        data[0U] = 0x1234560000000000;
        data[1U] = 0x4321;
        BitMemoryRegion region(data.data(), 40U, 40U);
        ss << region;
        ASSERT_EQ(ss.str(), "0x4321123456");
    }

    {
        clear();
        data[0U] = 0x123456789abcdef0;
        BitMemoryRegion region(data.data(), 2U, 20U);
        ss << region;
        ASSERT_EQ(ss.str(), "0xf37bc");
    }

    {
        clear();
        data[0U] = 0x123456789abcdef0;
        data[1U] = 0xfedcba9876543210;
        BitMemoryRegion region(data.data(), 16U, 96U);
        ss << region;
        ASSERT_EQ(ss.str(), "0xba9876543210123456789abc");
    }

    {
        clear();
        data[0U] = 0x1111111111111111;
        data[1U] = 0x2222222222222222;
        data[2U] = 0x4444444444444444;
        BitMemoryRegion region(data.data(), 31U, 120U);
        ss << region;
        ASSERT_EQ(ss.str(), "0x888888444444444444444422222222");
    }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
