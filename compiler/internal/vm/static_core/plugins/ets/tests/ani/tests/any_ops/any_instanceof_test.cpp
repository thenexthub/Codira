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

class AnyInstanceOfTest : public AniTest {
public:
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
TEST_F(AnyInstanceOfTest, AnyInstanceof_InValid)
{
    const int32_t longStringSize = 10000U;
    std::string longString(longStringSize, 'a');
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(longString.c_str(), longString.size(), &strRef), ANI_OK);
    ani_class strType {};
    ani_class stdCoreObj {};
    ASSERT_EQ(env_->FindClass("std.core.String", &strType), ANI_OK);
    ASSERT_EQ(env_->FindClass("std.core.Object", &stdCoreObj), ANI_OK);
    ani_boolean instanceof = ANI_FALSE;
    ASSERT_EQ(env_->Any_InstanceOf(strRef, stdCoreObj, & instanceof), ANI_OK);
    // NOTE: Test for Any_InstanceOf will be added after resolve #27087
    ASSERT_EQ(instanceof, ANI_FALSE);
}

TEST_F(AnyInstanceOfTest, AnyInstanceOf_InvalidArgs)
{
    ani_boolean result = ANI_FALSE;

    ani_class strType {};
    ASSERT_EQ(env_->FindClass("std.core.String", &strType), ANI_OK);
    ASSERT_EQ(env_->Any_InstanceOf(nullptr, strType, &result), ANI_INVALID_ARGS);

    ani_string strRef {};
    std::string_view val = "test";
    ASSERT_EQ(env_->String_NewUTF8(val.data(), val.size(), &strRef), ANI_OK);

    ASSERT_EQ(env_->Any_InstanceOf(strRef, nullptr, &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_InstanceOf(strRef, strType, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_InstanceOf(nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
}  // namespace ark::ets::ani::testing
