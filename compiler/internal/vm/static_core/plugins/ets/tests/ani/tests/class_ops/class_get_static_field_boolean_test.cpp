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

class ClassGetStaticFieldBooleanTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_boolean result = ANI_TRUE;
        ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
        ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
};

TEST_F(ClassGetStaticFieldBooleanTest, get_boolean)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassGetStaticFieldBooleanTest, get_boolean_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassGetStaticFieldBooleanTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldBooleanTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldBooleanTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldBooleanTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldBooleanTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldBooleanTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ani_boolean single = ANI_FALSE;
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "special1", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special4", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special5", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special6", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special7", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special8", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special9", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldBooleanTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    ani_boolean single = ANI_FALSE;
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_FALSE), ANI_OK);

    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldBooleanTest, combination_test2)
{
    ani_class cls {};
    ani_static_field field {};
    ani_boolean single = ANI_FALSE;
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBoolean", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum - 1; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, ANI_TRUE);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_FALSE), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldBooleanTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_boolean_test.TestBooleanA", "boolean_value");
}

TEST_F(ClassGetStaticFieldBooleanTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_boolean_test.TestBooleanFinal", "boolean_value");
}

TEST_F(ClassGetStaticFieldBooleanTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_boolean_test.TestBooleanFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "boolean_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_boolean_test.TestBooleanFinal"));
    ani_boolean boolValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &boolValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_boolean_test.TestBooleanFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
