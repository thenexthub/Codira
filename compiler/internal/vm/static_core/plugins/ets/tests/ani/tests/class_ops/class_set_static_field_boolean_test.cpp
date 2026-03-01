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
class ClassSetStaticFieldBooleanTest : public AniTest {
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

TEST_F(ClassSetStaticFieldBooleanTest, set_boolean)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldBooleanTest, set_boolean_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Boolean(env_, cls, field, ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldBooleanTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldBooleanTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(nullptr, field, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldBooleanTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldBooleanTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Boolean(nullptr, cls, field, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldBooleanTest, special_values)
{
    ani_class cls {};
    ani_static_field field {};
    ani_boolean single;
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ani_boolean(0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ani_boolean(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ani_boolean(!'s')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ani_boolean(!0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, true), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, false), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassSetStaticFieldBooleanTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    ani_boolean single;
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_FALSE), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, ANI_FALSE);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldBooleanTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_boolean_test.TestSetBoolean", "bool_value");
}

TEST_F(ClassSetStaticFieldBooleanTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_boolean_test.TestSetBooleanA", "bool_value");
}

TEST_F(ClassSetStaticFieldBooleanTest, combination_test4)
{
    CheckFieldValue("class_set_static_field_boolean_test.TestSetBooleanFinal", "bool_value");
}

TEST_F(ClassSetStaticFieldBooleanTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_boolean_test.TestSetBoolean", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_boolean_test.TestSetBoolean"));
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_FALSE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_boolean_test.TestSetBoolean"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)