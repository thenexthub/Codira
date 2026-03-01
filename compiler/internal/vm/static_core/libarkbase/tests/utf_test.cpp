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

#include "libarkbase/utils/utf.h"

#include <cstdint>

#include <vector>

#include <gtest/gtest.h>

namespace ark::utf::test {

TEST(Utf, ConvertMUtf8ToUtf16Double)
{
    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xedU, 0xa0U, 0x81U, 0xedU, 0xb0U, 0xb7U, 0x00U};
        const std::vector<uint16_t> res {0xd801U, 0xdc37U};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    {
        const std::vector<uint8_t> in {0x5bU, 0x61U, 0x62U, 0x63U, 0xedU, 0xa3U, 0x92U, 0x5dU, 0x00U};
        const std::vector<uint16_t> res {0x5bU, 0x61U, 0x62U, 0x63U, 0xd8d2U, 0x5dU};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    {
        const std::vector<uint8_t> in {0xF0U, 0x9FU, 0x91U, 0xB3U, 0x00U};
        const std::vector<uint16_t> res {0xD83DU, 0xDC73U};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }
}

TEST(Utf, ConvertMUtf8ToUtf16)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint8_t> in {0xc0U, 0x80U, 0x00U};
        const std::vector<uint16_t> res {0x0U};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> in {0x7fU, 0x00U};
        const std::vector<uint16_t> res {0x7fU};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xc2U, 0xa7U, 0x33U, 0x00U};
        const std::vector<uint16_t> res {0xa7U, 0x33U};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xefU, 0xbfU, 0x83U, 0x33U, 0x00U};
        const std::vector<uint16_t> res {0xffc3U, 0x33U};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }
}

TEST(Utf, Utf16ToMUtf8Size)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint16_t> in {0x0U};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 3U);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint16_t> in {0x7fU};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xa7U, 0x33U};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 4U);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xffc3U, 0x33U};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 5U);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xd801U, 0xdc37U};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 5U);
    }
}

TEST(Utf, ConvertRegionUtf16ToMUtf8_1b)
{
    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint16_t> in {0x7fU};
        const std::vector<uint8_t> res {0x7fU, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 1U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }
}

TEST(Utf, ConvertRegionUtf16ToMUtf8_2b)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint16_t> in {0x0U};
        const std::vector<uint8_t> res {0xc0U, 0x80U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 2U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xa7U, 0x33U};
        const std::vector<uint8_t> res {0xc2U, 0xa7U, 0x33U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 3U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }
}

TEST(Utf, ConvertRegionUtf16ToMUtf8_3b)
{
    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xffc3U, 0x33U};
        const std::vector<uint8_t> res {0xefU, 0xbfU, 0x83U, 0x33U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    // utf-16 data in 0xd800-0xdfff
    {
        const std::vector<uint16_t> in {0xd834U, 0x33U};
        const std::vector<uint8_t> res {0xedU, 0xa0U, 0xb4U, 0x33U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    // utf-16 data in 0xd800-0xdfff
    {
        const std::vector<uint16_t> in {0xdf06U, 0x33U};
        const std::vector<uint8_t> res {0xedU, 0xbcU, 0x86U, 0x33U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xd801U, 0xdc37U};
        const std::vector<uint8_t> res {0xf0U, 0x90U, 0x90U, 0xb7U, 0x00U};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1L, 0U);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1L] = '\0';
        EXPECT_EQ(out, res);
    }
}

TEST(Utf, ConvertUtf16ToUtf8IsolatedHighSurrogate)
{
    uint16_t lead = 0xD834U;
    uint16_t nonTrail = 0x0041U;  // 'A', not a trail surrogate

    auto res = ConvertUtf16ToUtf8(lead, nonTrail, false);
    EXPECT_EQ(res.n, UtfLength::THREE);
    EXPECT_EQ(res.ch[0], 0xEFU);
    EXPECT_EQ(res.ch[1], 0xBFU);
    EXPECT_EQ(res.ch[2], 0xBDU);

    auto mutfRes = ConvertUtf16ToUtf8(lead, nonTrail, true);
    EXPECT_EQ(mutfRes.n, UtfLength::THREE);
    EXPECT_EQ(mutfRes.ch[0], 0xEFU);
    EXPECT_EQ(mutfRes.ch[1], 0xBFU);
    EXPECT_EQ(mutfRes.ch[2], 0xBDU);
}

TEST(Utf, CompareMUtf8ToMUtf8_1b)
{
    // 1-byte utf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> v1 {0x00U};
        const std::vector<uint8_t> v2 {0x7fU, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x02U, 0x00U};
        const std::vector<uint8_t> v2 {0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x7fU, 0x00U};
        const std::vector<uint8_t> v2 {0x7fU, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0x01U, 0x7fU, 0x00U};
        const std::vector<uint8_t> v2 {0x01U, 0x70U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x01U, 0x71U, 0x00U};
        const std::vector<uint8_t> v2 {0x01U, 0x73U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0L);
    }
}

TEST(Utf, CompareMUtf8ToMUtf8_2b)
{
    // 2-byte utf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xdfU, 0xbfU, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xdfU, 0xbfU, 0x03U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0xdfU, 0xb1U, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xd1U, 0xb2U, 0x03U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xd1U, 0xbfU, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xdfU, 0xb0U, 0x03U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0L);
    }
}

TEST(Utf, CompareMUtf8ToMUtf8_3b)
{
    // 3-byte utf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xefU, 0xbfU, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xefU, 0xbfU, 0x03U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0xefU, 0xb2U, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xe0U, 0xbfU, 0x03U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xefU, 0xb0U, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xefU, 0xbfU, 0x05U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0L);
    }
}

TEST(Utf, CompareMUtf8ToMUtf8_4b)
{
    // 4-byte utf-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x0aU, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf8U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0L);
    }
}

TEST(Utf, CompareUtf8ToUtf8_1b)
{
    // 1-byte utf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> v1 {0x00U};
        const std::vector<uint8_t> v2 {0x7fU, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x02U, 0x00U};
        const std::vector<uint8_t> v2 {0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x7fU, 0x00U};
        const std::vector<uint8_t> v2 {0x7fU, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0x01U, 0x7fU, 0x00U};
        const std::vector<uint8_t> v2 {0x01U, 0x70U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0x01U, 0x71U, 0x00U};
        const std::vector<uint8_t> v2 {0x01U, 0x73U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0L);
    }
}

TEST(Utf, CompareUtf8ToUtf8_2b)
{
    // 2-byte utf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xdfU, 0xbfU, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xdfU, 0xbfU, 0x03U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xdfU, 0xb1U, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xd1U, 0xb2U, 0x03U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xd1U, 0xbfU, 0x03U, 0x00U};
        const std::vector<uint8_t> v2 {0xdfU, 0xb0U, 0x03U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0L);
    }
}

TEST(Utf, CompareUtf8ToUtf8_3b)
{
    // 3-byte utf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xefU, 0xbfU, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xefU, 0xbfU, 0x03U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0xefU, 0xb2U, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xe0U, 0xbfU, 0x03U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xefU, 0xb0U, 0x03U, 0x04U, 0x00U};
        const std::vector<uint8_t> v2 {0xefU, 0xbfU, 0x05U, 0x04U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0L);
    }
}

TEST(Utf, CompareUtf8ToUtf8_4b)
{
    // 4-byte utf-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0U);
    }

    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x0aU, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0L);
    }

    {
        const std::vector<uint8_t> v1 {0xf7U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        const std::vector<uint8_t> v2 {0xf8U, 0xbfU, 0xbfU, 0x04U, 0x05U, 0x00U};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0L);
    }
}

TEST(Utf, IsValidUTF8)
{
    // test one byte
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0x7F};  // 01111111
        EXPECT_TRUE(IsValidUTF8(data));
    }
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0x80};  // 10000000
        EXPECT_FALSE(IsValidUTF8(data));
    }
    // test two byte
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0xC2, 0x80};  // 11000010 10000000
        EXPECT_TRUE(IsValidUTF8(data));
    }
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0x80, 0x80};  // 10000000 10000000
        EXPECT_FALSE(IsValidUTF8(data));
    }
    // test three byte
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0xE0, 0xA0, 0x80};  // 11100000 10100000 10000000
        EXPECT_TRUE(IsValidUTF8(data));
    }
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0xC0, 0x80, 0x80};  // 11000000 10000000 10000000
        EXPECT_FALSE(IsValidUTF8(data));
    }
    // test four byte
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0xF0, 0x90, 0x80, 0x80};  // 11110000 10010000 10000000 10000000
        EXPECT_TRUE(IsValidUTF8(data));
    }
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0xE0, 0x90, 0x80, 0x80};  // 11100000 10010000 10000000 10000000
        EXPECT_FALSE(IsValidUTF8(data));
    }
}
}  // namespace ark::utf::test
