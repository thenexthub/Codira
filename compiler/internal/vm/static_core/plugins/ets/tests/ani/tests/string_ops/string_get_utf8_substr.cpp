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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class StringGetUtf8SubStringTest : public AniTest {};

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SubstrSizeMismatchMultiByte)
{
    const std::string example {"🙂🙂"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 7U;
    ani_size result = 0U;
    const ani_size utfBufferSize = 30U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, utfBufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, substrSize);
    ASSERT_STREQ(utfBuffer, "🙂");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SubstrContainEnd)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {1U};
    const uint32_t scope = 9U;
    for (auto i = 0U; i < scope; ++i) {
        utfBuffer[i] = 1;
    }
    const uint32_t strEnd = 9U;
    utfBuffer[strEnd] = '\0';
    ani_size substrOffset = 0U;
    ani_size substrSize = example.size();
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "example");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SpecialString)
{
    const std::string example {"hi\n, \rtest\\?"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 20U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 1U;
    const ani_size substrSize = 10U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "i\n, \rtest\\");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_EmptyString)
{
    // NOLINTNEXTLINE(readability-redundant-string-init)
    const std::string example {""};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, 0U, 0U, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 0U);
    ASSERT_STREQ(utfBuffer, "");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullEnv)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    auto status2 = env_->c_api->String_GetUTF8SubString(nullptr, string, 0U, 0U, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status2, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullString)
{
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    ani_status status =
        env_->String_GetUTF8SubString(nullptr, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullBuffer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    const ani_size bufferSize = 100U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, nullptr, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullResultPointer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_ValidSubstring)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "exam");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_OffsetNegative)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = -1U;
    ani_size substrSize = 2U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_OffsetOutOfRange)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 10U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SizeOutOfRange)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 34U;
    ani_size substrSize = std::numeric_limits<uint32_t>::max() - 8U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer,
                                           std::numeric_limits<uint32_t>::max(), &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SizeOutOfBounds)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    const ani_size substrSize = 10U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_EmptySubstring)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 0U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SubstrSizeNegative)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 0U;
    ani_size substrSize = -1U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferEmpty)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 1U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 0U;
    ani_size substrSize = 1U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferSpecial)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {'\n', '\r', 'h', 'i', '\\', '\0'};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 0U;
    ani_size substrSize = 3U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "exa");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferSizeNegative)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, -1U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "examp");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferSizeZero)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, 0U, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferTooSmall)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 3U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_Repeat)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(result, substrSize);
        ASSERT_STREQ(utfBuffer, "exam");
    }
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_ComUtf8)
{
    const std::string testStr = "Hello, 世界";
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(testStr.c_str(), testStr.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(string, nullptr);

    ani_size result2 = 0U;
    auto status2 = env_->String_GetUTF8Size(string, &result2);
    ASSERT_EQ(status2, ANI_OK);
    ASSERT_EQ(result2, testStr.size());

    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};
    ani_size resultSize = 0U;
    auto status3 = env_->String_GetUTF8(string, utfBuffer, bufferSize, &resultSize);
    ASSERT_EQ(status3, ANI_OK);
    ASSERT_EQ(resultSize, testStr.size());
    ASSERT_STREQ(utfBuffer, "Hello, 世界");

    const ani_size bufferSize4 = 10U;
    char utfBuffer4[bufferSize4] = {0U};
    ani_size offset = 2U;
    const ani_size subSize = 3U;
    ani_size resultSize4 = 0U;
    auto status4 = env_->String_GetUTF8SubString(string, offset, subSize, utfBuffer4, bufferSize4, &resultSize4);
    ASSERT_EQ(status4, ANI_OK);
    ASSERT_EQ(resultSize4, subSize);
    ASSERT_STREQ(utfBuffer4, "llo");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_ComGetUtf8SubString)
{
    const std::string testStr = "Hello World";
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(testStr.c_str(), testStr.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(string, nullptr);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};
    ani_size offset = 6U;
    ani_size subSize = 5U;
    ani_size resultSize = 0U;
    env_->String_GetUTF8SubString(string, offset, subSize, utfBuffer, sizeof(utfBuffer), &resultSize);
    ASSERT_STREQ(utfBuffer, "World");
    ASSERT_EQ(resultSize, subSize);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)