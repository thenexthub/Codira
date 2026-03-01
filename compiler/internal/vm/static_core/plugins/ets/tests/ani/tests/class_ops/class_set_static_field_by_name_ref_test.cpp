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

class ClassSetStaticFieldByNameRefTest : public AniTest {
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

// NOTE: Enable when #22354 is resolved
TEST_F(ClassSetStaticFieldByNameRefTest, static_field_by_name_ref_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Ref(env_, cls, "string_value", string), ANI_OK);
    ani_ref resultValue = nullptr;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Ref(env_, cls, "string_value", &resultValue), ANI_OK);

    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 7U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 6U);
    ASSERT_STREQ(buffer.data(), "abcdef");
}

TEST_F(ClassSetStaticFieldByNameRefTest, static_field_by_name_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", string), ANI_OK);
    ani_ref resultValue = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_value", &resultValue), ANI_OK);

    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 7U> buffer {};
    ani_size resSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 6U);
    ASSERT_STREQ(buffer.data(), "abcdef");
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "int_value", string), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_object)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(nullptr, "string_value", string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_field)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, nullptr, string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_value)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);

    ani_string string {};
    ani_string string2 {};
    ani_size resSize = 0U;
    ani_ref resultValue = nullptr;
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8("fedcba", 6U, &string2), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", string), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_value", &resultValue), ANI_OK);
        auto name = static_cast<ani_string>(resultValue);
        std::array<char, 30U> buffer {};
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
        ASSERT_STREQ(buffer.data(), "abcdef");
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", string2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_value", &resultValue), ANI_OK);
    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 30U> buffer {};
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_STREQ(buffer.data(), "fedcba");
}

TEST_F(ClassSetStaticFieldByNameRefTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_ref_test.BoxStaticA", "string_value");
}

TEST_F(ClassSetStaticFieldByNameRefTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_ref_test.BoxStaticFinal", "string_value");
}

TEST_F(ClassSetStaticFieldByNameRefTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStatic", &cls), ANI_OK);
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "", string), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "\n", string), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Ref(nullptr, cls, "string_value", string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_ref_test.BoxStaticFinal", &cls), ANI_OK);

    ani_string stringValue {};
    ASSERT_EQ(env_->String_NewUTF8("test", 4U, &stringValue), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_ref_test.BoxStaticFinal"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_valuex", stringValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_ref_test.BoxStaticFinal"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", stringValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_ref_test.BoxStaticFinal"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
