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

class StringNewUtf8Test : public AniTest {};
TEST_F(StringNewUtf8Test, StringNewUtf8_NullUtf8String)
{
    ani_string result = nullptr;
    const ani_size size = 10U;
    auto status = env_->String_NewUTF8(nullptr, size, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    ASSERT_EQ(result, nullptr);
}

TEST_F(StringNewUtf8Test, StringNewUtf8_EmptyString)
{
    ani_string result = nullptr;
    auto status = env_->String_NewUTF8("", 0U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf8Test, StringNewUtf8_LongString)
{
    const int32_t longStringSize = 10000U;
    std::string longString(longStringSize, 'a');
    ani_string result = nullptr;
    auto status = env_->String_NewUTF8(longString.c_str(), longString.size(), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf8Test, StringNewUtf8_ValidMultibyteString)
{
    const std::string example {"测测试emoji🙂🙂"};
    ani_string result = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_string result2 = nullptr;
    status = env_->String_NewUTF8(example.c_str(), example.size(), &result2);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};
    ani_size resultSize = 0U;
    env_->String_GetUTF8SubString(result2, 0U, example.size(), utfBuffer, sizeof(utfBuffer), &resultSize);
    ASSERT_EQ(resultSize, example.size());
    ASSERT_STREQ(utfBuffer, example.c_str());
}

TEST_F(StringNewUtf8Test, StringNewUtf8_NullEnv)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->c_api->String_NewUTF8(nullptr, example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringNewUtf8Test, StringNewUtf8_SmallerSize)
{
    const std::string example {"example"};
    ani_string result = nullptr;
    ani_status status = env_->String_NewUTF8(example.c_str(), example.size() - 1U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_string result2 = nullptr;
    status = env_->String_NewUTF8(example.c_str(), example.size(), &result2);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};
    ani_size resultSize = 0U;
    env_->String_GetUTF8SubString(result2, 0U, example.size() - 1U, utfBuffer, sizeof(utfBuffer), &resultSize);
    ASSERT_STREQ(utfBuffer, "exampl");
}

TEST_F(StringNewUtf8Test, StringNewUtf8_ZeroSize)
{
    const std::string example {"example"};
    ani_string result = nullptr;
    ani_status status = env_->String_NewUTF8(example.c_str(), 0U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_string result2 = nullptr;
    status = env_->String_NewUTF8(example.c_str(), example.size(), &result2);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
    const uint32_t bufferSize = 20U;
    char utfBuffer[bufferSize] = {0U};
    ani_size resultSize = 0U;
    env_->String_GetUTF8SubString(result2, 0U, 0U, utfBuffer, sizeof(utfBuffer), &resultSize);
    ASSERT_STREQ(utfBuffer, "");
}

TEST_F(StringNewUtf8Test, StringNewUtf8_Repeat)
{
    const std::string example {"测测试emoji🙂🙂"};
    ani_string result = nullptr;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        auto status = env_->String_NewUTF8(example.c_str(), example.size(), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_NE(result, nullptr);
    }
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)