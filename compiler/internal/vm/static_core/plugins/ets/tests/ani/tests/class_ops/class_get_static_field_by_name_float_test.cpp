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

class ClassGetStaticFieldByNameFloatTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_float setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, fieldName, setTarget), ANI_OK);
        ani_float resultValue = 0.0;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameFloatTest, get_static_field_float_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float age = 0.0F;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Float(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_float>(20.0F));
}

TEST_F(ClassGetStaticFieldByNameFloatTest, get_static_field_float)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float age = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_float>(20.0F));
}

TEST_F(ClassGetStaticFieldByNameFloatTest, get_static_field_float_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);

    ani_float age = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float age = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float age = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float age = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Float(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    ani_float single = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "specia1", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "specia5", &single), ANI_INVALID_TYPE);
    ani_float max = std::numeric_limits<ani_float>::max();
    ani_float minpositive = std::numeric_limits<ani_float>::min();
    ani_float min = -std::numeric_limits<ani_float>::max();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "floatMin", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(min));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "minpositive", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(minpositive));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "floatMax", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(max));
}

TEST_F(ClassGetStaticFieldByNameFloatTest, combination_test1)
{
    ani_class cls {};
    const ani_float expectedAge = 2.0F;
    const ani_float expectedAge2 = 3.0F;
    ani_float single = 0.0F;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "age", expectedAge2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, expectedAge2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "age", expectedAge), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, expectedAge);
}

TEST_F(ClassGetStaticFieldByNameFloatTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_float_test.GetFloatStatic", "age");
}

TEST_F(ClassGetStaticFieldByNameFloatTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_float_test.FloatStaticA", "float_value");
}

TEST_F(ClassGetStaticFieldByNameFloatTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_float_test.FloatStaticFinal", "float_value");
}

TEST_F(ClassGetStaticFieldByNameFloatTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_float_test.GetFloatStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_float_test.GetFloatStatic"));
    ani_float floatValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "agex", &floatValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_float_test.GetFloatStatic"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "age", &floatValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_float_test.GetFloatStatic"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
