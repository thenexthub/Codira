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

class ClassGetStaticFieldByNameShortTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_short setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, fieldName, setTarget), ANI_OK);
        ani_short resultValue = 1U;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short age = 1U;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Short(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_short>(20U));
}

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short age = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_short>(20U));
}

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);

    ani_short age = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short age = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short age = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short age = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Short(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    ani_short single = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "specia1", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "specia5", &single), ANI_INVALID_TYPE);
    ani_short max = std::numeric_limits<ani_short>::max();
    ani_short min = std::numeric_limits<ani_short>::min();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "min", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(min));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "max", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(max));
}

TEST_F(ClassGetStaticFieldByNameShortTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    const ani_short setTarget2 = 3U;
    ani_short single = 1U;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.GetShortStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "age", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "age", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameShortTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_short_test.GetShortStatic", "age");
}

TEST_F(ClassGetStaticFieldByNameShortTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_short_test.ShortStaticA", "short_value");
}

TEST_F(ClassGetStaticFieldByNameShortTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_short_test.ShortStaticFinal", "short_value");
}

TEST_F(ClassGetStaticFieldByNameShortTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_short_test.ShortStaticA", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_short_test.ShortStaticA"));
    ani_short shortValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_valuex", &shortValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_short_test.ShortStaticA"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &shortValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_short_test.ShortStaticA"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
