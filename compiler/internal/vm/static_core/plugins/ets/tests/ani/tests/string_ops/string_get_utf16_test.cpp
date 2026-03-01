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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers,
// cppcoreguidelines-pro-bounds-pointer-arithmetic)
namespace ark::ets::ani::testing {

class StringGetUtf16Test : public AniTest {};

TEST_F(StringGetUtf16Test, StringGetUTF16_NullUtf16String)
{
    ani_string string = nullptr;
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    auto status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_NullEnv)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    auto status2 = env_->c_api->String_GetUTF16(nullptr, string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status2, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_EmptyString)
{
    const uint16_t example[] = {0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, 0U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_SpecialString)
{
    const std::string example {"hello\nworld\r, hi\\?"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 30U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    for (ani_size i = 0; i < result; ++i) {
        ASSERT_EQ(utf16Buffer[i], example[i]);
    }
}

TEST_F(StringGetUtf16Test, StringGetUTF16_AsciiString)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 5U);
    ASSERT_EQ(utf16Buffer[4U], 0x006F);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_AsciiString1)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 6U);
    ASSERT_EQ(utf16Buffer[5U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_NonAsciiString)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 6U);
}

TEST_F(StringGetUtf16Test, StringGetutf16_NullResult)
{
    const uint16_t example[] = {0x4F60};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 1U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf16Test, StringGetutf16_BufferEmpty)
{
    const uint16_t example[] = {0x4F60};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 1U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferBound1)
{
    const uint16_t example[] = {0x4F60};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 2U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 1U);
    ASSERT_EQ(utf16Buffer[0U], 0x4F60);
    ASSERT_EQ(utf16Buffer[1U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferBound2)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 8U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 7U);
    ASSERT_EQ(utf16Buffer[6U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string {};
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 6U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall1)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    uint16_t utf16Buffer[3U] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, 0U, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall2)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 3U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetutf16Repeat)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string = nullptr;
    ani_size exampleSize = sizeof(example) / sizeof(uint16_t) - 1U;
    auto status = env_->String_NewUTF16(example, exampleSize, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 30U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(result, exampleSize);
    }
}

TEST_F(StringGetUtf16Test, StringGetUtf16ComUtf8ToUtf16)
{
    const std::string testUtf8Str = "Hello, 世界🙂";
    ani_string utf8String = nullptr;
    ani_string utf16String = nullptr;

    auto status = env_->String_NewUTF8(testUtf8Str.c_str(), testUtf8Str.size(), &utf8String);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(utf8String, nullptr);

    const uint32_t bufferSize = 30U;
    char utf8Buffer[bufferSize] = {0U};
    ani_size resultSize = 0U;
    auto status1 = env_->String_GetUTF8(utf8String, utf8Buffer, bufferSize, &resultSize);
    ASSERT_EQ(status1, ANI_OK);
    ASSERT_STREQ(utf8Buffer, "Hello, 世界🙂");
    ASSERT_EQ(resultSize, testUtf8Str.size());

    std::u16string utf16Converted(reinterpret_cast<const char16_t *>(utf8Buffer),
                                  reinterpret_cast<const char16_t *>(utf8Buffer + (resultSize + 1)));
    ASSERT_STREQ(reinterpret_cast<const char *>(utf16Converted.c_str()), "Hello, 世界🙂");
    auto status2 = env_->String_NewUTF16(reinterpret_cast<const uint16_t *>(utf16Converted.c_str()),
                                         utf16Converted.size(), &utf16String);
    ASSERT_EQ(status2, ANI_OK);
    ASSERT_NE(utf16String, nullptr);

    const ani_size buffer16Size = 30U;
    uint16_t utf16Buffer[buffer16Size] = {0U};
    ani_size result2 = 0U;
    auto status3 = env_->String_GetUTF16(utf16String, utf16Buffer, buffer16Size, &result2);
    ASSERT_EQ(status3, ANI_OK);
    ASSERT_EQ(result2, utf16Converted.size());
    ASSERT_STREQ(reinterpret_cast<const char *>(utf16Buffer), "Hello, 世界🙂");
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers,
// cppcoreguidelines-pro-bounds-pointer-arithmetic)