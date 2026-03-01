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

class ClassSetStaticFieldCharTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_char result = '\0';
        const ani_char target = 'a';
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_char setTar = 'b';
        ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassSetStaticFieldCharTest, set_char)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
    const ani_char target = 'a';
    ASSERT_EQ(result, target);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldCharTest, set_char_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Char(env_, cls, field, &result), ANI_OK);
    const ani_char target = 'a';
    ASSERT_EQ(result, target);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Char(env_, cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Char(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldCharTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, setTar), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldCharTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char result = '\0';
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &result), ANI_OK);
    const ani_char target = 'a';
    ASSERT_EQ(result, target);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->Class_SetStaticField_Char(nullptr, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldCharTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, nullptr, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldCharTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_char setTar = 'c';
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Char(nullptr, cls, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldCharTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_char single = 'c';
    ani_char maxValue = std::numeric_limits<ani_char>::max();
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, maxValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, maxValue);

    ani_char minValue = std::numeric_limits<ani_char>::min();
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, minValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Char(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minValue);
}

TEST_F(ClassSetStaticFieldCharTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_char setTarget = 'c';
    const ani_char setTarget2 = 'b';
    ani_char single = '\0';
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetChar", &cls), ANI_OK);
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

TEST_F(ClassSetStaticFieldCharTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_char_test.TestSetChar", "char_value");
}

TEST_F(ClassSetStaticFieldCharTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_char_test.TestSetCharA", "char_value");
}

TEST_F(ClassSetStaticFieldCharTest, combination_test4)
{
    CheckFieldValue("class_set_static_field_char_test.TestSetCharFinal", "char_value");
}

TEST_F(ClassSetStaticFieldCharTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_char_test.TestSetCharFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "char_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_char_test.TestSetCharFinal"));
    const ani_char charValue = 'a';
    ASSERT_EQ(env_->Class_SetStaticField_Char(cls, field, charValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_char_test.TestSetCharFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
