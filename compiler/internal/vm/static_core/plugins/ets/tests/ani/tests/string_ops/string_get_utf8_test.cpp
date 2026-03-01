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

class StringGetUtf8StringTest : public AniTest {};

TEST_F(StringGetUtf8StringTest, StringGetUtf8_MultiByte)
{
    const std::string example {"🙂🙂"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "🙂🙂");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_BasicString)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "example");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_EmptyString)
{
    // NOLINTNEXTLINE(readability-redundant-string-init)
    const std::string example {""};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_EmptyBuffer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 1U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_SpecialBuffer)
{
    const std::string example {"h\ni\r\\!"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "h\ni\r\\!");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_BufferSizeZero)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 1U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, 0U, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullEnv)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->c_api->String_GetUTF8(nullptr, string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullString)
{
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_status status = env_->String_GetUTF8(nullptr, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullBuffer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    const ani_size bufferSize = 100U;
    status = env_->String_GetUTF8(string, nullptr, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullResultPointer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_BufferTooSmall)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 3U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_Managed)
{
    const auto string = static_cast<ani_string>(CallEtsFunction<ani_ref>("string_get_utf8_test", "GetString"));
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_status status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 4U);
    ASSERT_STREQ(utfBuffer, "test");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_Repeat)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(result, example.size());
        ASSERT_STREQ(utfBuffer, "example");
    }
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_ZeroSymbol)
{
    const size_t strSize = 11U;
    const std::string example("abc\0def\0ghi", strSize);
    ani_string aniString = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), strSize, &aniString);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    status = env_->String_GetUTF16Size(aniString, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());

    ani_size result2 = 0U;
    const ani_size bufferSize = 20U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    status = env_->String_GetUTF16(aniString, utf16Buffer, bufferSize, &result2);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result2, example.size());

    ani_size resultUTF8 = 0U;
    status = env_->String_GetUTF8Size(aniString, &resultUTF8);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(resultUTF8, example.size());

    ani_size result2UTF8 = 0U;
    char utf8Buffer[bufferSize] = {0U};
    status = env_->String_GetUTF8(aniString, utf8Buffer, bufferSize, &result2UTF8);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result2UTF8, example.size());

    for (size_t i = 0; i < strSize; ++i) {
        ASSERT_EQ(example[i], utf16Buffer[i]);
        ASSERT_EQ(example[i], utf8Buffer[i]);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
