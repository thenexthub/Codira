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

#include "common_components/base/utf_helper.h"
#include "common_components/tests/test_helper.h"

using namespace common;
namespace common::test {
class UtfHelperTest : public common::test::BaseTestWithScope {
};

HWTEST_F_L0(UtfHelperTest, DecodeUTF16Test1)
{
    uint16_t utf16[] = {0xD7FF};
    size_t index = 0;
    size_t len = 1;
    uint32_t result = utf_helper::DecodeUTF16(utf16, len, &index, false);
    EXPECT_EQ(result, 0xD7FF);

    uint16_t utf16In[] = {0xDC00};
    result = utf_helper::DecodeUTF16(utf16In, len, &index, false);
    EXPECT_EQ(result, 0xDC00);

    uint16_t utf16In1[] = {0xD800};
    result = utf_helper::DecodeUTF16(utf16In1, len, &index, false);
    EXPECT_EQ(result, 0xD800);

    uint16_t utf16In2[] = {0xD7FF};
    len = 2;
    result = utf_helper::DecodeUTF16(utf16In2, len, &index, false);
    EXPECT_EQ(result, 0xD7FF);

    len = 1;
    result = utf_helper::DecodeUTF16(utf16In2, len, &index, false);
    EXPECT_EQ(result, 0xD7FF);

    result = utf_helper::DecodeUTF16(utf16In, len, &index, false);
    EXPECT_EQ(result, 0xDC00);
}

HWTEST_F_L0(UtfHelperTest, DecodeUTF16Test2)
{
    size_t index = 0;
    uint16_t utf16[] = {0xD800, 0xDC00};
    size_t len = 2;
    utf16[1] = 0xFFFF;
    uint32_t result = utf_helper::DecodeUTF16(utf16, len, &index, false);
    EXPECT_EQ(result, 0xD800);
}

HWTEST_F_L0(UtfHelperTest, DecodeUTF16Test3)
{
    size_t index = 0;
    uint16_t utf16[] = {0xD800, 0xDC00};
    size_t len = 2;
    uint32_t result = utf_helper::DecodeUTF16(utf16, len, &index, true);
    EXPECT_EQ(result, 0xD800);

    uint16_t utf16In[] = {0xD800, 0x0041};
    result = utf_helper::DecodeUTF16(utf16In, len, &index, false);
    EXPECT_EQ(result, 0xD800);
    
    uint16_t utf16In1[] = {0xD800, 0xDC00};
    index = 0;
    len = 2;
    result = utf_helper::DecodeUTF16(utf16In1, len, &index, false);
    EXPECT_EQ(result, 0x10000);

    uint16_t utf16In2[] = {0x0041, 0x0042};
    index = 0;
    len = 2;
    result = utf_helper::DecodeUTF16(utf16In2, len, &index, false);
    EXPECT_EQ(result, 0x0041);
}

HWTEST_F_L0(UtfHelperTest, HandleAndDecodeInvalidUTF16Test1)
{
    uint16_t input[] = {0xDC00};
    size_t index = 0;
    size_t len = sizeof(input) / sizeof(input[0]);
    uint32_t result = utf_helper::HandleAndDecodeInvalidUTF16(input, len, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);

    uint16_t input1[] = {0xD800, 0x0041};
    index = 0;
    len = sizeof(input1) / sizeof(input1[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input1, len, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);

    uint16_t input2[] = {0x0041};
    index = 0;
    len = sizeof(input2) / sizeof(input2[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input2, len, &index);
    EXPECT_EQ(result, 0x0041);

    uint16_t input3[] = {0xDBFF};
    index = 0;
    len = sizeof(input3) / sizeof(input3[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input3, len, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);

    uint16_t input4[] = {0xDFFF};
    index = 0;
    len = sizeof(input4) / sizeof(input4[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input4, len, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);
}

HWTEST_F_L0(UtfHelperTest, HandleAndDecodeInvalidUTF16Test2)
{
    uint16_t input[] = {0xD800};
    size_t index = 0;
    size_t len = sizeof(input) / sizeof(input[0]);
    uint32_t result = utf_helper::HandleAndDecodeInvalidUTF16(input, len, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);

    uint16_t input1[] = {0xD800, 0xD800};
    size_t len1 = sizeof(input1) / sizeof(input1[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input1, len1, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);

    uint16_t input2[] = {'A'};
    size_t len2 = sizeof(input2) / sizeof(input2[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input2, len2, &index);
    EXPECT_EQ(result, 'A');

    uint16_t input3[] = {0xD800 ^ 0x01};
    size_t len3 = sizeof(input3) / sizeof(input3[0]);
    result = utf_helper::HandleAndDecodeInvalidUTF16(input3, len3, &index);
    EXPECT_EQ(result, utf_helper::UTF16_REPLACEMENT_CHARACTER);
}

HWTEST_F_L0(UtfHelperTest, HandleAndDecodeInvalidUTF16Test3)
{
    uint16_t input[] = {0xDBFF, 0xDFFF};
    size_t index = 0;
    size_t len = sizeof(input) / sizeof(input[0]);
    uint32_t expected = ((0xDBFF - utf_helper::DECODE_LEAD_LOW) << utf_helper::UTF16_OFFSET) +
        (0xDFFF - utf_helper::DECODE_TRAIL_LOW) + utf_helper::DECODE_SECOND_FACTOR;
    uint32_t result = utf_helper::HandleAndDecodeInvalidUTF16(input, len, &index);
    EXPECT_EQ(result, expected);
}

HWTEST_F_L0(UtfHelperTest, HandleAndDecodeInvalidUTF16Test4)
{
    uint16_t input[] = {0xD800, 0xDC00};
    size_t index = 0;
    size_t len = sizeof(input) / sizeof(input[0]);
    uint32_t expected = ((0xD800 - utf_helper::DECODE_LEAD_LOW) << utf_helper::UTF16_OFFSET) +
        (0xDC00 - utf_helper::DECODE_TRAIL_LOW) + utf_helper::DECODE_SECOND_FACTOR;
    uint32_t result = utf_helper::HandleAndDecodeInvalidUTF16(input, len, &index);
    EXPECT_EQ(result, expected);
    EXPECT_EQ(index, 1);
}

HWTEST_F_L0(UtfHelperTest, IsValidUTF8Test1)
{
    std::vector<uint8_t> data = {0xED, 0xA0, 0x80};
    EXPECT_FALSE(utf_helper::IsValidUTF8(data));

    std::vector<uint8_t> data1 = {0xED, 0x90, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data1));

    std::vector<uint8_t> data2 = {0xED, 0xC0, 0x80};
    EXPECT_FALSE(utf_helper::IsValidUTF8(data2));

    std::vector<uint8_t> data3 = {0xED, 0x80, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data3));

    std::vector<uint8_t> data4 = {0xE0, 0xA0, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data4));

    std::vector<uint8_t> data5 = {0xC2, 0xA9}; // © symbol
    EXPECT_TRUE(utf_helper::IsValidUTF8(data5));

    std::vector<uint8_t> data6 = {0xE2, 0x82, 0xAC}; // € symbol
    EXPECT_TRUE(utf_helper::IsValidUTF8(data6));
}

HWTEST_F_L0(UtfHelperTest, IsValidUTF8Test2)
{
    std::vector<uint8_t> data = {0xF4, 0x90, 0x80, 0x80};
    EXPECT_FALSE(utf_helper::IsValidUTF8(data));

    std::vector<uint8_t> data1 = {0xF5, 0x80, 0x80, 0x80};
    EXPECT_FALSE(utf_helper::IsValidUTF8(data1));

    std::vector<uint8_t> data2 = {0xF0, 0x90, 0x80, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data2));

    std::vector<uint8_t> data3 = {0xF1, 0x80, 0x80, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data3));

    std::vector<uint8_t> data4 = {0xF4, 0x80, 0x80, 0x80};
    EXPECT_TRUE(utf_helper::IsValidUTF8(data4));
}

HWTEST_F_L0(UtfHelperTest, ConvertRegionUtf16ToUtf8Test3)
{
    uint8_t utf8Out[10];
    size_t result = utf_helper::ConvertRegionUtf16ToUtf8(nullptr, utf8Out, 5, 10, 0, false, false, false);
    EXPECT_EQ(result, 0);

    uint16_t utf16In[] = {0x0041};
    result = utf_helper::ConvertRegionUtf16ToUtf8(utf16In, nullptr, 1, sizeof(utf16In), 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(utf16In, utf8Out, 1, 0, 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(nullptr, utf8Out, 1, 0, 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(utf16In, nullptr, 1, 0, 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(nullptr, nullptr, 1, sizeof(utf8Out), 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(nullptr, nullptr, 1, 0, 0, false, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToUtf8(utf16In, utf8Out, 1, sizeof(utf8Out), 0, false, false, false);
    EXPECT_EQ(result, 1);
}

HWTEST_F_L0(UtfHelperTest, ConvertRegionUtf16ToLatin1Test)
{
    uint8_t utf8Out[10];
    size_t result = utf_helper::ConvertRegionUtf16ToLatin1(nullptr, utf8Out, 5, 10);
    EXPECT_EQ(result, 0);

    uint16_t utf16In[] = {0x0041};
    result = utf_helper::ConvertRegionUtf16ToLatin1(utf16In, nullptr, 1, sizeof(utf16In));
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(utf16In, utf8Out, 1, 0);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(nullptr, utf8Out, 1, 0);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(utf16In, nullptr, 1, 0);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(nullptr, nullptr, 1, sizeof(utf8Out));
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(nullptr, nullptr, 1, 0);
    EXPECT_EQ(result, 0);

    result = utf_helper::ConvertRegionUtf16ToLatin1(utf16In, utf8Out, 1, sizeof(utf8Out));
    EXPECT_EQ(result, 1);

    const uint16_t input[] = {0x0041, 0x0042, 0x0043};
    uint8_t output[2] = {0};
    result = utf_helper::ConvertRegionUtf16ToLatin1(input, output, 3, 2);
    EXPECT_EQ(result, 2);
}

HWTEST_F_L0(UtfHelperTest, DebuggerConvertRegionUtf16ToUtf8Test)
{
    uint8_t utf8Out[10];
    size_t result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(nullptr, utf8Out, 5, sizeof(utf8Out), 0, false, false);
    EXPECT_EQ(result, 0);

    uint16_t utf16In[] = {0x0041};
    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(utf16In, nullptr, 1, sizeof(utf16In), 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(nullptr, nullptr, 1, sizeof(utf8Out), 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(nullptr, utf8Out, 1, 0, 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(nullptr, utf8Out, 1, 0, 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(nullptr, nullptr, 1, 0, 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(utf16In, utf8Out, 1, 0, 0, false, false);
    EXPECT_EQ(result, 0);

    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(utf16In, utf8Out, 1, sizeof(utf8Out), 0, false, false);
    EXPECT_EQ(result, 1);

    uint16_t utf16In1[] = {0x0041, 0x0042, 0x0043};
    result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(utf16In1, nullptr, 3, 0, 0, false, false);
    EXPECT_EQ(result, 0);
}

HWTEST_F_L0(UtfHelperTest, DecodeUTF16Test4)
{
    uint16_t utf16[] = {0xD800, 0xDC00};
    size_t index = 0;
    size_t len = sizeof(utf16) / sizeof(utf16[0]);

    uint32_t result = utf_helper::DecodeUTF16(utf16, len, &index, false);
    EXPECT_EQ(result, 0x10000);
    EXPECT_EQ(index, 1);
}

HWTEST_F_L0(UtfHelperTest, Utf8ToUtf16Size_LastByteIsTwoByteStart_TrimLastByte)
{
    const uint8_t utf8[] = {0xC0};
    size_t utf8Len = sizeof(utf8);

    size_t result = utf_helper::Utf8ToUtf16Size(utf8, utf8Len);
    EXPECT_EQ(result, 1);
}

HWTEST_F_L0(UtfHelperTest, Utf8ToUtf16Size_LastTwoBytesStartWithThreeByteHeader_TrimTwoBytes)
{
    const uint8_t utf8[] = {0xE2, 0x82};
    size_t utf8Len = sizeof(utf8);

    size_t result = utf_helper::Utf8ToUtf16Size(utf8, utf8Len);
    EXPECT_EQ(result, 2);
}

HWTEST_F_L0(UtfHelperTest, Utf8ToUtf16Size_IncompleteSequenceAtEnd_ReturnsSizeWithoutInvalid)
{
    const uint8_t utf8[] = {0xF0, 0x90, 0x8D};
    size_t result = utf_helper::Utf8ToUtf16Size(utf8, 3);
    EXPECT_EQ(result, 3);
}

HWTEST_F_L0(UtfHelperTest, DebuggerConvertRegionUtf16ToUtf8_ZeroCodepoint_WriteNulChar)
{
    uint16_t utf16In[] = {0x0000};
    uint8_t utf8Out[10] = {0};
    size_t utf16Len = 1;
    size_t utf8Len = sizeof(utf8Out);
    size_t start = 0;
    bool modify = false;
    bool isWriteBuffer = true;

    size_t result = utf_helper::DebuggerConvertRegionUtf16ToUtf8(
        utf16In, utf8Out, utf16Len, utf8Len, start, modify, isWriteBuffer);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(utf8Out[0], 0x00U);
}

HWTEST_F_L0(UtfHelperTest, ConvertUtf16ToUtf8_NulChar_WriteBufferMode_ReturnsZeroByte)
{
    uint16_t d0 = 0x0000;
    uint16_t d1 = 0;
    bool modify = false;
    bool isWriteBuffer = true;

    utf_helper::Utf8Char result = utf_helper::ConvertUtf16ToUtf8(d0, d1, modify, isWriteBuffer);

    EXPECT_EQ(result.n, 1);
    EXPECT_EQ(result.ch[0], 0x00U);
}
} // namespace common::test