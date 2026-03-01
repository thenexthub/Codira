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
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class AnyCallMethodTest : public AniTest {
protected:
    void CheckANIStr(const ani_string &str, const std::string &expected) const
    {
        const ani_size utfBufferSize = 20;
        std::array<char, utfBufferSize> utfBuffer = {0};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(env_->String_GetUTF8SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(),
                                                &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer.data(), expected.c_str());
    }
};

TEST_F(AnyCallMethodTest, AnyCallMethod_Invalid)
{
    const char *clsName = "any_call_method_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);
    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);
    ani_ref nameRef {};
    // NOTE: Test for Any_CallMethod will be added after resolve #27087
    ASSERT_EQ(env_->Any_CallMethod(static_cast<ani_ref>(res), "test", 0, nullptr, &nameRef), ANI_PENDING_ERROR);
}

TEST_F(AnyCallMethodTest, AnyCallMethod_NullArgs)
{
    const char *validName = "test";
    auto dummyRef = reinterpret_cast<ani_ref>(0x1234);
    ani_ref resultRef {};

    ASSERT_EQ(env_->Any_CallMethod(nullptr, validName, 0U, nullptr, &resultRef), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_CallMethod(dummyRef, nullptr, 0U, nullptr, &resultRef), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_CallMethod(dummyRef, validName, 0U, nullptr, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_CallMethod(dummyRef, validName, 2U, nullptr, &resultRef), ANI_INVALID_ARGS);
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)

}  // namespace ark::ets::ani::testing
