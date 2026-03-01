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

class ClassGetStaticFieldByNameIntTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_int setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, fieldName, setTarget), ANI_OK);
        ani_int resultValue = 0;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int age = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Int(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_int>(20U));
}

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int age = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_int>(20U));
}

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);

    ani_int age = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int age = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int age = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int age = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Int(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    ani_int single = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "specia1", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(0));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "specia5", &single), ANI_INVALID_TYPE);
    ani_int max = std::numeric_limits<ani_int>::max();
    ani_int min = std::numeric_limits<ani_int>::min();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "min", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(min));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "max", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(max));
}

TEST_F(ClassGetStaticFieldByNameIntTest, combination_test1)
{
    ani_class cls {};
    const ani_int setTarget = 2U;
    const ani_int setTarget2 = 3U;
    ani_int single = 0;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "age", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "age", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameIntTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_int_test.GetIntStatic", "age");
}

TEST_F(ClassGetStaticFieldByNameIntTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_int_test.PackstaticA", "int_value");
}

TEST_F(ClassGetStaticFieldByNameIntTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_int_test.PackstaticFinal", "int_value");
}

TEST_F(ClassGetStaticFieldByNameIntTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_int_test.GetIntStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_int_test.GetIntStatic"));
    ani_int intValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "agex", &intValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_int_test.GetIntStatic"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "age", &intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_int_test.GetIntStatic"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
