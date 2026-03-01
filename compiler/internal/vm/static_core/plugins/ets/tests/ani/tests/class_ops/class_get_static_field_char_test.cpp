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
class ClassGetStaticFieldCharTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_char result = 0U;
        const ani_char target = 'c';
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_char setTar = 'a';
        ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassGetStaticFieldCharTest, get_char)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    const ani_char target = 'c';
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldCharTest, get_char_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    const ani_char target = 'c';
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Char(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldCharTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldCharTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->Class_GetStaticField_Char(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldCharTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_char result = '\0';
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldCharTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldCharTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Char(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldCharTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ani_char single = '\0';
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "special1", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special2", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_INVALID_TYPE);

    std::array<char, 5U> expectedValues = {'\n', '\r', '\t', '\b', '\0'};
    for (size_t i = 0; i < expectedValues.size(); ++i) {
        std::string fieldName = "special" + std::to_string(i + 4);
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName.c_str(), &field), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, expectedValues[i]);
    }

    const ani_char maxTarget = std::numeric_limits<ani_char>::max();
    const ani_char minTarget = std::numeric_limits<ani_char>::min();
    ASSERT_EQ(env_->Class_FindStaticField(cls, "charMax", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, maxTarget);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "charMin", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minTarget);
}

TEST_F(ClassGetStaticFieldCharTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_char setTarget = 'a';
    const ani_char setTarget2 = 'b';
    ani_char single;
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum - 1; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTarget), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, setTarget);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTarget2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, setTarget2);
}

TEST_F(ClassGetStaticFieldCharTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_char_test.TestCharA", "char_value");
}

TEST_F(ClassGetStaticFieldCharTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_char_test.TestCharFinal", "char_value");
}

TEST_F(ClassGetStaticFieldCharTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_char_test.TestChar", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_char_test.TestChar"));
    ani_char charValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &charValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_char_test.TestChar"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
