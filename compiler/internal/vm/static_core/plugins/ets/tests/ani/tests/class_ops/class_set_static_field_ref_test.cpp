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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class ClassSetStaticFieldRefTest : public AniTest {
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

TEST_F(ClassSetStaticFieldRefTest, set_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string), ANI_OK);
    ani_ref resultValue = nullptr;
    ASSERT_EQ(env_->Class_GetStaticField_Ref(cls, field, &resultValue), ANI_OK);
    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 11U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 4U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 4U);
    ASSERT_STREQ(buffer.data(), "test");
}

TEST_F(ClassSetStaticFieldRefTest, set_ref_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticField_Ref(env_, cls, field, string), ANI_OK);
    ani_ref resultValue = nullptr;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Ref(env_, cls, field, &resultValue), ANI_OK);
    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 11U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 4U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 4U);
    ASSERT_STREQ(buffer.data(), "test");
}

TEST_F(ClassSetStaticFieldRefTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldRefTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticField_Ref(nullptr, field, string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldRefTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, nullptr, string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldRefTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticField_Ref(nullptr, cls, field, string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldRefTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRef", &cls), ANI_OK);
    ani_static_field field {};
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("testString", 10U, &string), ANI_OK);
    ani_string string1 {};
    ASSERT_EQ(env_->String_NewUTF8("String", 6U, &string1), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ani_size resSize = 0U;

    ASSERT_NE(field, nullptr);
    ani_ref resultValue = nullptr;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
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

TEST_F(ClassSetStaticFieldRefTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_ref_test.TestSetRefA", "string_value");
}

TEST_F(ClassSetStaticFieldRefTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_ref_test.TestSetRefFinal", "string_value");
}

TEST_F(ClassSetStaticFieldRefTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_ref_test.TestSetRefA", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("test", 4U, &string), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_ref_test.TestSetRefA"));
    ASSERT_EQ(env_->Class_SetStaticField_Ref(cls, field, string), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_ref_test.TestSetRefA"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
