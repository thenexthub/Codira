/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class AnyNewTest : public AniTest {
public:
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
TEST_F(AnyNewTest, AnyNew_InvalidType)
{
    const int32_t longStringSize = 10000U;
    std::string longString(longStringSize, 'a');
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(longString.c_str(), longString.size(), &strRef), ANI_OK);
    ani_ref anyStringRef {};
    ASSERT_EQ(env_->Any_New(strRef, 0U, nullptr, &anyStringRef), ANI_PENDING_ERROR);
    // NOTE: Test for Any_New will be added after resolve #27087
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(AnyNewTest, AnyNew_InvalidParams)
{
    ani_ref dummy {};
    ani_ref result {};
    ASSERT_EQ(env_->Any_New(nullptr, 0U, nullptr, &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_New(dummy, 0U, nullptr, nullptr), ANI_INVALID_ARGS);

    std::array<ani_ref, 1U> args {};
    ASSERT_EQ(env_->Any_New(dummy, 0U, args.data(), &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_New(dummy, 1U, nullptr, &result), ANI_INVALID_ARGS);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)

}  // namespace ark::ets::ani::testing
