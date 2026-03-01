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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldFloatTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_float result = 0U;
        const ani_float target = 18.0F;
        ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_float setTar = 20.0F;
        ASSERT_EQ(env_->Class_SetStaticField_Float(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassGetStaticFieldFloatTest, get_float)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    const ani_float target = 18.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldFloatTest, get_float_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    const ani_float target = 18.0F;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Float(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldFloatTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Float(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    ani_float single = 0.0F;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia1", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia4", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia5", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_INVALID_TYPE);
    ani_float max = std::numeric_limits<ani_float>::max();
    ani_float minpositive = std::numeric_limits<ani_float>::min();
    ani_float min = -std::numeric_limits<ani_float>::max();
    ASSERT_EQ(env_->Class_FindStaticField(cls, "floatMin", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "floatMax", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "minpositive", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minpositive);
}

TEST_F(ClassGetStaticFieldFloatTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloat", &cls), ANI_OK);
    ani_static_field field {};
    const ani_float setTar = 28.0F;
    const ani_float setTar2 = 18.0F;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Float(cls, field, setTar2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar2);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Float(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassGetStaticFieldFloatTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_float_test.TestFloat", "float_value");
}

TEST_F(ClassGetStaticFieldFloatTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_float_test.TestFloatA", "float_value");
}

TEST_F(ClassGetStaticFieldFloatTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_float_test.TestFloatFinal", "float_value");
}

TEST_F(ClassGetStaticFieldFloatTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_float_test.TestFloatFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_float_test.TestFloatFinal"));
    ani_float floatValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &floatValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_float_test.TestFloatFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
