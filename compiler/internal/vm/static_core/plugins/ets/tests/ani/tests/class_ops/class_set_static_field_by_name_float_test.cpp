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

namespace ark::ets::ani::testing {

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
class ClassSetStaticFieldByNameFloatTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_capi)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Float(env_, cls, "float_value", setTarget), ANI_OK);
    ani_float resultValue = 0.0F;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Float(env_, cls, "float_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", setTarget), ANI_OK);
    ani_float resultValue = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_field_type)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "string_value", setTarget), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_args_object)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(nullptr, "float_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_args_field)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, nullptr, setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", static_cast<ani_float>(0)), ANI_OK);
    ani_float single = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(0));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", static_cast<ani_float>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(NULL));

    ani_float max = std::numeric_limits<ani_float>::max();
    ani_float minpositive = std::numeric_limits<ani_float>::min();
    ani_float min = -std::numeric_limits<ani_float>::max();

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", static_cast<ani_float>(max)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(max));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", static_cast<ani_float>(minpositive)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(minpositive));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", static_cast<ani_float>(min)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_float>(min));
}

TEST_F(ClassSetStaticFieldByNameFloatTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    const ani_short setTarget2 = 3.0F;
    ani_float single = 0.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_float_test.FloatStaticA", "float_value");
}

TEST_F(ClassSetStaticFieldByNameFloatTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_float_test.FloatStaticFinal", "float_value");
}

TEST_F(ClassSetStaticFieldByNameFloatTest, invalid_argument1)
{
    ani_class cls {};
    const ani_short setTarget = 2.0F;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Float(nullptr, cls, "float_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_float_test.FloatStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_float_test.FloatStatic"));
    const ani_float floatValue = 2.2;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_valuex", floatValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_float_test.FloatStatic"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", floatValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_float_test.FloatStatic"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
