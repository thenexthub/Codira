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

class StringGetUtf8SizeTest : public AniTest {};
TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_NullString)
{
    ani_size result = 0U;
    ani_status status = env_->String_GetUTF8Size(nullptr, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_NullEnv)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    ani_size result = 0U;
    auto status2 = env_->c_api->String_GetUTF8Size(nullptr, string, &result);
    ASSERT_EQ(status2, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_NullResultPointer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    status = env_->String_GetUTF8Size(string, nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_ValidString)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    status = env_->String_GetUTF8Size(string, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_ValidMultibyteString)
{
    const std::string example = {"测测试emoji🙂🙂"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    status = env_->String_GetUTF8Size(string, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_EmptyString)
{
    // NOLINTNEXTLINE(readability-redundant-string-init)
    const std::string example {""};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    status = env_->String_GetUTF8Size(string, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
}

TEST_F(StringGetUtf8SizeTest, StringGetUtf8Size_Repeat)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    ani_size result = 0U;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        status = env_->String_GetUTF8Size(string, &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(result, example.size());
    }
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)