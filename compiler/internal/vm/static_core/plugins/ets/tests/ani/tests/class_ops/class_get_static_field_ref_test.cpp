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
#include <cmath>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,  readability-magic-numbers)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldRefTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string), ANI_OK);
        ani_ref resultValue;
        ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &resultValue), ANI_OK);
        auto name = static_cast<ani_string>(resultValue);
        std::array<char, 11U> buffer {};
        ani_size resSize = 0U;
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 4U, buffer.data(), buffer.size(), &resSize), ANI_OK);
        ASSERT_EQ(resSize, 4U);
        ASSERT_STREQ(buffer.data(), "test");
    }
};

TEST_F(ClassGetStaticFieldRefTest, get_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &result), ANI_OK);
    auto getRes = static_cast<ani_string>(result);
    std::array<char, 11U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(getRes, 0U, 10U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 10U);
    ASSERT_STREQ(buffer.data(), "testString");
}

TEST_F(ClassGetStaticFieldRefTest, get_ref_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Ref(env_, cls, field, &result), ANI_OK);
    auto getRes = static_cast<ani_string>(result);
    std::array<char, 11U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(getRes, 0U, 10U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 10U);
    ASSERT_STREQ(buffer.data(), "testString");
}

TEST_F(ClassGetStaticFieldRefTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldRefTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->Class_GetStaticField_Ref(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldRefTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldRefTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldRefTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Ref(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldRefTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRef", &cls), ANI_OK);
    ani_static_field field {};
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);
    ani_string string1 {};
    ASSERT_EQ(env_->String_NewUTF8("String", 6U, &string1), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_ref resultValue = nullptr;
    ani_size resSize = 0U;
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string1), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &resultValue), ANI_OK);
        auto name = static_cast<ani_string>(resultValue);
        std::array<char, 30U> buffer {};
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 4U, buffer.data(), buffer.size(), &resSize), ANI_OK);
        ASSERT_STREQ(buffer.data(), "Stri");
    }
    ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &resultValue), ANI_OK);
    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 30U> buffer {};
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 4U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_STREQ(buffer.data(), "test");
}

TEST_F(ClassGetStaticFieldRefTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_ref_test.TestRef", "string_value");
}

TEST_F(ClassGetStaticFieldRefTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_ref_test.TestRefA", "string_value");
}

TEST_F(ClassGetStaticFieldRefTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_ref_test.TestRefFinal", "string_value");
}

TEST_F(ClassGetStaticFieldRefTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_ref_test.TestRefA", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_ref_test.TestRefA"));
    ani_ref refValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &refValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_ref_test.TestRefA"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
