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

class ClassGetStaticFieldByNameRefTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, fieldName, string), ANI_OK);
        ani_ref resultValue = nullptr;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, fieldName, &resultValue), ANI_OK);

        auto name = static_cast<ani_string>(resultValue);
        std::array<char, 7U> buffer {};
        ani_size resSize = 0U;
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
        ASSERT_EQ(resSize, 6U);
        ASSERT_STREQ(buffer.data(), "abcdef");
    }
};

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);

    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Ref(env_, cls, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Bob");
}

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);

    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Bob");
}

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "age", &nameRef), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(nullptr, "name", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, nullptr, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ani_ref nameRef = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "", &nameRef), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "\n", &nameRef), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Ref(nullptr, cls, "name", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.Man", &cls), ANI_OK);
    ani_ref resultValue = nullptr;
    ani_string string {};
    ani_string string2 {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8("fedcba", 6U, &string2), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount - 1; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "name", string), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", &resultValue), ANI_OK);
        auto name = static_cast<ani_string>(resultValue);
        std::array<char, 30U> buffer {};
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
        ASSERT_EQ(resSize, 6U);
        ASSERT_STREQ(buffer.data(), "abcdef");
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "name", string2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", &resultValue), ANI_OK);
    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 30U> buffer {};
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 6U);
    ASSERT_STREQ(buffer.data(), "fedcba");
}

TEST_F(ClassGetStaticFieldByNameRefTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_ref_test.Man", "name");
}

TEST_F(ClassGetStaticFieldByNameRefTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_ref_test.BoxStaticA", "string_value");
}

TEST_F(ClassGetStaticFieldByNameRefTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_ref_test.BoxStaticFinal", "string_value");
}

TEST_F(ClassGetStaticFieldByNameRefTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_ref_test.BoxStaticA", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_ref_test.BoxStaticA"));
    ani_ref refValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_valuex", &refValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_ref_test.BoxStaticA"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_value", &refValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_ref_test.BoxStaticA"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
