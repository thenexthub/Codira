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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class StringNewUtf16Test : public AniTest {};

TEST_F(StringNewUtf16Test, StringNewUtf16_NullUtf16String)
{
    ani_string result = nullptr;
    const ani_size size = 10U;
    auto status = env_->String_NewUTF16(nullptr, size, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    ASSERT_EQ(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_EmptyString)
{
    ani_string result = nullptr;
    const uint16_t stringTest[] = {0x0000};
    auto status = env_->String_NewUTF16(stringTest, 0U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_NullEnv)
{
    ani_string result = nullptr;
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    size_t length = sizeof(example) / sizeof(example[0U]) - 1U;
    auto status = env_->c_api->String_NewUTF16(nullptr, example, length, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_AsciiString1)
{
    ani_string result = nullptr;
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    size_t length = sizeof(example) / sizeof(example[0U]) - 1U;
    auto status = env_->String_NewUTF16(example, length, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_AsciiString2)
{
    ani_string result = nullptr;
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    size_t length = sizeof(example) / sizeof(example[0U]);
    auto status = env_->String_NewUTF16(example, length, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_NonAsciiString)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string result = nullptr;
    size_t length = sizeof(example) / sizeof(example[0U]) - 1U;
    auto status = env_->String_NewUTF16(example, length, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_MixedString)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x4F60, 0x0021, 0x0000};
    ani_string result = nullptr;
    size_t length = sizeof(example) / sizeof(example[0U]) - 1U;
    auto status = env_->String_NewUTF16(example, length, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_TooLongString)
{
    const uint16_t strSize = 1000U;
    uint16_t example[strSize] = {0x0000};
    std::fill(std::begin(example), std::end(example), 0x0048);
    ani_string result = nullptr;
    auto status = env_->String_NewUTF16(example, strSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(StringNewUtf16Test, StringNewUtf16_Repeat)
{
    ani_string result = nullptr;
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    size_t length = sizeof(example) / sizeof(example[0U]);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        auto status = env_->String_NewUTF16(example, length, &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_NE(result, nullptr);
    }
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
