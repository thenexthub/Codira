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

class ClassGetStaticFieldByNameLongTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_long setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, fieldName, setTarget), ANI_OK);
        ani_long resultValue = 0L;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameLongTest, get_static_field_long_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long age = 0L;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Long(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_long>(20L));
}

TEST_F(ClassGetStaticFieldByNameLongTest, get_static_field_long)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long age = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_long>(20L));
}

TEST_F(ClassGetStaticFieldByNameLongTest, get_static_field_long_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);

    ani_long age = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameLongTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long age = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameLongTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long age = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameLongTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameLongTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long age = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Long(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameLongTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    ani_long single = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "specia1", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "specia5", &single), ANI_INVALID_TYPE);
    ani_long max = std::numeric_limits<ani_long>::max();
    ani_long min = std::numeric_limits<ani_long>::min();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "min", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(min));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "max", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(max));
}

TEST_F(ClassGetStaticFieldByNameLongTest, combination_test1)
{
    ani_class cls {};
    const ani_long setTarget = 2L;
    const ani_long setTarget2 = 3L;
    ani_long single = 0L;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "age", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "age", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameLongTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_long_test.GetLongStatic", "age");
}

TEST_F(ClassGetStaticFieldByNameLongTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_long_test.PackageStaticA", "long_value");
}

TEST_F(ClassGetStaticFieldByNameLongTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_long_test.PackageStaticFinal", "long_value");
}

TEST_F(ClassGetStaticFieldByNameLongTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_long_test.GetLongStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_long_test.GetLongStatic"));
    ani_long longValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "agex", &longValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_long_test.GetLongStatic"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "age", &longValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_long_test.GetLongStatic"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
