/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "utils/utf.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include <gtest/gtest.h>

namespace panda::utf::test {

HWTEST(Utf, ConvertMUtf8ToUtf16_1, testing::ext::TestSize.Level0)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint8_t> in {0xc0, 0x80, 0x00};
        const std::vector<uint16_t> res {0x0};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> in {0x7f, 0x00};
        const std::vector<uint16_t> res {0x7f};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xc2, 0xa7, 0x33, 0x00};
        const std::vector<uint16_t> res {0xa7, 0x33};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xef, 0xbf, 0x83, 0x33, 0x00};
        const std::vector<uint16_t> res {0xffc3, 0x33};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }
}

// double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
HWTEST(Utf, ConvertMUtf8ToUtf16_2, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> in {0xed, 0xa0, 0x81, 0xed, 0xb0, 0xb7, 0x00};
        const std::vector<uint16_t> res {0xd801, 0xdc37};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    {
        const std::vector<uint8_t> in {0x5b, 0x61, 0x62, 0x63, 0xed, 0xa3, 0x92, 0x5d, 0x00};
        const std::vector<uint16_t> res {0x5b, 0x61, 0x62, 0x63, 0xd8d2, 0x5d};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }

    {
        const std::vector<uint8_t> in {0xF0, 0x9F, 0x91, 0xB3, 0x00};
        const std::vector<uint16_t> res {0xD83D, 0xDC73};
        std::vector<uint16_t> out(res.size());
        ConvertMUtf8ToUtf16(in.data(), utf::Mutf8Size(in.data()), out.data());
        EXPECT_EQ(out, res);
    }
}

/**
 * @tc.name: MUtf8ToUtf16Size
 * @tc.desc: Verify the MUtf8ToUtf16Size function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(Utf, MUtf8ToUtf16Size, testing::ext::TestSize.Level0)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint8_t> in {0xc0, 0x80};
        size_t res = MUtf8ToUtf16Size(in.data(), in.size());
        EXPECT_EQ(res, 1U);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> in {0x7f};
        size_t res = MUtf8ToUtf16Size(in.data(), in.size());
        EXPECT_EQ(res, 1U);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xc2, 0xa7, 0x33};
        size_t res = MUtf8ToUtf16Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xef, 0xbf, 0x83, 0x33};
        size_t res = MUtf8ToUtf16Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xed, 0xa0, 0x81, 0xed, 0xb0, 0xb7};
        size_t res = MUtf8ToUtf16Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }
}

HWTEST(Utf, Utf16ToMUtf8Size, testing::ext::TestSize.Level0)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint16_t> in {0x0};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 3U);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint16_t> in {0x7f};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }

    {
        const std::vector<uint16_t> in {0x7f};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 2U);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xa7, 0x33};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 4U);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xffc3, 0x33};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 5U);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xd801, 0xdc37};
        size_t res = Utf16ToMUtf8Size(in.data(), in.size());
        EXPECT_EQ(res, 5U);
    }
}

/**
 * @tc.name: ConvertRegionMUtf8ToUtf16
 * @tc.desc: Verify the ConvertRegionMUtf8ToUtf16 function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(Utf, ConvertRegionMUtf8ToUtf16, testing::ext::TestSize.Level0)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint8_t> in {0xc0, 0x80, 0x00};
        const std::vector<uint16_t> res {0x0};
        std::vector<uint16_t> out(res.size());
        ConvertRegionMUtf8ToUtf16(in.data(), out.data(), utf::Mutf8Size(in.data()), res.size() * sizeof(uint16_t), 0);
        EXPECT_EQ(out, res);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint8_t> in {0x7f, 0x00};
        const std::vector<uint16_t> res {0x7f};
        std::vector<uint16_t> out(res.size());
        ConvertRegionMUtf8ToUtf16(in.data(), out.data(), utf::Mutf8Size(in.data()), res.size() * sizeof(uint16_t), 0);
        EXPECT_EQ(out, res);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xc2, 0xa7, 0x33, 0x00};
        const std::vector<uint16_t> res {0xa7, 0x33};
        std::vector<uint16_t> out(res.size());
        ConvertRegionMUtf8ToUtf16(in.data(), out.data(), utf::Mutf8Size(in.data()), res.size() * sizeof(uint16_t), 0);
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xef, 0xbf, 0x83, 0x33, 0x00};
        const std::vector<uint16_t> res {0xffc3, 0x33};
        std::vector<uint16_t> out(res.size());
        ConvertRegionMUtf8ToUtf16(in.data(), out.data(), utf::Mutf8Size(in.data()), res.size() * sizeof(uint16_t), 0);
        EXPECT_EQ(out, res);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint8_t> in {0xed, 0xa0, 0x81, 0xed, 0xb0, 0xb7, 0x00};
        const std::vector<uint16_t> res1 {0xd801, 0xdc37};
        const std::vector<uint16_t> res2 {0xdc37};
        std::vector<uint16_t> out1(res1.size());
        std::vector<uint16_t> out2(res2.size());
        size_t in_len1 = utf::Mutf8Size(in.data());
        size_t in_len2 = utf::Mutf8Size(in.data() + 3);
        ConvertRegionMUtf8ToUtf16(in.data(), out1.data(), in_len1, res1.size() * sizeof(uint16_t), 0);
        ConvertRegionMUtf8ToUtf16(in.data(), out2.data(), in_len1, res2.size() * sizeof(uint16_t), in_len1 - in_len2);
        EXPECT_EQ(out1, res1);
        EXPECT_EQ(out2, res2);
    }
}

HWTEST(Utf, ConvertRegionUtf16ToMUtf8_1, testing::ext::TestSize.Level0)
{
    // 2-byte mutf-8 U+0000
    {
        const std::vector<uint16_t> in {0x0};
        const std::vector<uint8_t> res {0xc0, 0x80, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 2U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }

    // 1-byte mutf-8: 0xxxxxxx
    {
        const std::vector<uint16_t> in {0x7f};
        const std::vector<uint8_t> res {0x7f, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 1U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }

    // 2-byte mutf-8: 110xxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xa7, 0x33};
        const std::vector<uint8_t> res {0xc2, 0xa7, 0x33, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 3U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xffc3, 0x33};
        const std::vector<uint8_t> res {0xef, 0xbf, 0x83, 0x33, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }
}

HWTEST(Utf, ConvertRegionUtf16ToMUtf8_2, testing::ext::TestSize.Level0)
{
    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    // utf-16 data in 0xd800-0xdfff
    {
        const std::vector<uint16_t> in {0xd834, 0x33};
        const std::vector<uint8_t> res {0xed, 0xa0, 0xb4, 0x33, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }

    // 3-byte mutf-8: 1110xxxx 10xxxxxx 10xxxxxx
    // utf-16 data in 0xd800-0xdfff
    {
        const std::vector<uint16_t> in {0xdf06, 0x33};
        const std::vector<uint8_t> res {0xed, 0xbc, 0x86, 0x33, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }

    // double 3-byte mutf-8: 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
    {
        const std::vector<uint16_t> in {0xd801, 0xdc37};
        const std::vector<uint8_t> res {0xf0, 0x90, 0x90, 0xb7, 0x00};
        std::vector<uint8_t> out(res.size());
        size_t sz = ConvertRegionUtf16ToMUtf8(in.data(), out.data(), in.size(), out.size() - 1, 0);
        EXPECT_EQ(sz, 4U);
        out[out.size() - 1] = '\0';
        EXPECT_EQ(out, res);
    }
}

// 1-byte utf-8: 0xxxxxxx
HWTEST(Utf, CompareMUtf8ToMUtf8_1, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0);
    }

    {
        const std::vector<uint8_t> v1 {0x02, 0x00};
        const std::vector<uint8_t> v2 {0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0x01, 0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x01, 0x70, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0x01, 0x71, 0x00};
        const std::vector<uint8_t> v2 {0x01, 0x73, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0);
    }
}

// 2-byte utf-8: 110xxxxx 10xxxxxx
HWTEST(Utf, CompareMUtf8ToMUtf8_2, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xdf, 0xbf, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xdf, 0xbf, 0x03, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xdf, 0xb1, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xd1, 0xb2, 0x03, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xd1, 0xbf, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xdf, 0xb0, 0x03, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0);
    }
}

// 3-byte utf-8: 1110xxxx 10xxxxxx 10xxxxxx
HWTEST(Utf, CompareMUtf8ToMUtf8_3, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xef, 0xbf, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xef, 0xbf, 0x03, 0x04, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xef, 0xb2, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xe0, 0xbf, 0x03, 0x04, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xef, 0xb0, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xef, 0xbf, 0x05, 0x04, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0);
    }
}

// 4-byte utf-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
HWTEST(Utf, CompareMUtf8ToMUtf8_4, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x0a, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf8, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareMUtf8ToMUtf8(v1.data(), v2.data()) < 0);
    }
}

// 1-byte utf-8: 0xxxxxxx
HWTEST(Utf, CompareUtf8ToUtf8_1, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0);
    }

    {
        const std::vector<uint8_t> v1 {0x02, 0x00};
        const std::vector<uint8_t> v2 {0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0x01, 0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x01, 0x70, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0x01, 0x71, 0x00};
        const std::vector<uint8_t> v2 {0x01, 0x73, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0);
    }
}

// 2-byte utf-8: 110xxxxx 10xxxxxx
HWTEST(Utf, CompareUtf8ToUtf8_2, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xdf, 0xbf, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xdf, 0xbf, 0x03, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xdf, 0xb1, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xd1, 0xb2, 0x03, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xd1, 0xbf, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xdf, 0xb0, 0x03, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0);
    }
}

// 3-byte utf-8: 1110xxxx 10xxxxxx 10xxxxxx
HWTEST(Utf, CompareUtf8ToUtf8_3, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xef, 0xbf, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xef, 0xbf, 0x03, 0x04, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xef, 0xb2, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xe0, 0xbf, 0x03, 0x04, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xef, 0xb0, 0x03, 0x04, 0x00};
        const std::vector<uint8_t> v2 {0xef, 0xbf, 0x05, 0x04, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0);
    }
}

// 4-byte utf-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
HWTEST(Utf, CompareUtf8ToUtf8_4, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) == 0);
    }

    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x0a, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) > 0);
    }

    {
        const std::vector<uint8_t> v1 {0xf7, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        const std::vector<uint8_t> v2 {0xf8, 0xbf, 0xbf, 0x04, 0x05, 0x00};
        EXPECT_TRUE(CompareUtf8ToUtf8(v1.data(), v1.size(), v2.data(), v2.size()) < 0);
    }
}

HWTEST(Utf, IsMUtf8OnlySingleBytes, testing::ext::TestSize.Level0)
{
    const std::vector<uint8_t> v1 {0x02, 0x00};
    EXPECT_TRUE(IsMUtf8OnlySingleBytes(v1.data()));

    const std::vector<uint8_t> v2 {0x90, 0x00};
    EXPECT_FALSE(IsMUtf8OnlySingleBytes(v2.data()));
}

HWTEST(Utf, IsValidModifiedUTF8, testing::ext::TestSize.Level0)
{
    const std::vector<uint8_t> v1 {0x31, 0x00};
    EXPECT_TRUE(IsValidModifiedUTF8(v1.data()));

    const std::vector<uint8_t> v2 {0x9f, 0x00};
    EXPECT_FALSE(IsValidModifiedUTF8(v2.data()));

    const std::vector<uint8_t> v3 {0xf7, 0x00};
    EXPECT_FALSE(IsValidModifiedUTF8(v3.data()));

    const std::vector<uint8_t> v4 {0xe0, 0x00};
    EXPECT_FALSE(IsValidModifiedUTF8(v4.data()));

    const std::vector<uint8_t> v5 {0xd4, 0x00};
    EXPECT_FALSE(IsValidModifiedUTF8(v5.data()));

    const std::vector<uint8_t> v6 {0x11, 0x31, 0x00};
    EXPECT_TRUE(IsValidModifiedUTF8(v6.data()));

    const std::vector<uint8_t> v7 {0xf8, 0x00};
    EXPECT_FALSE(IsValidModifiedUTF8(v7.data()));
}

HWTEST(Utf, ConvertMUtf8ToUtf16Pair, testing::ext::TestSize.Level0)
{
    const uint8_t data = 0x11;
    std::pair<uint32_t, size_t> p1 = ConvertMUtf8ToUtf16Pair(&data, 2U);
    ASSERT_EQ(17U, p1.first);
    ASSERT_EQ(1U, p1.second);

    std::pair<uint32_t, size_t> p2 = ConvertMUtf8ToUtf16Pair(&data, 3U);
    ASSERT_EQ(17U, p2.first);
    ASSERT_EQ(1U, p2.second);
}

HWTEST(Utf, IsEqualTest, testing::ext::TestSize.Level0)
{
    {
        const std::vector<uint8_t> v1 {0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        Span<const uint8_t> utf8_1(v1.data(), v1.size());
        Span<const uint8_t> utf8_2(v2.data(), v2.size());
        ASSERT_TRUE(IsEqual(utf8_1, utf8_2));
    }

    {
        const std::vector<uint8_t> v1 {0x7f, 0x7f, 0x00};
        const std::vector<uint8_t> v2 {0x7f, 0x00};
        Span<const uint8_t> utf8_1(v1.data(), v1.size());
        Span<const uint8_t> utf8_2(v2.data(), v2.size());
        ASSERT_FALSE(IsEqual(utf8_1, utf8_2));
    }

    {
        const std::vector<uint8_t> v1 {0xdf, 0xbf, 0x03, 0x00};
        const std::vector<uint8_t> v2 {0xdf, 0xbf, 0x03, 0x00};
        EXPECT_TRUE(IsEqual(v1.data(), v2.data()));
    }
}

}  // namespace panda::utf::test
