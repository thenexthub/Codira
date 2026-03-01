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

class ClassSetStaticFieldByNameShortTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameShortTest, set_static_field_by_name_long_capi)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Short(env_, cls, "short_value", setTarget), ANI_OK);
    ani_short resultValue = 1U;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Short(env_, cls, "short_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameShortTest, set_static_field_by_name_long)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", setTarget), ANI_OK);
    ani_short resultValue = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameShortTest, set_static_field_by_name_long_invalid_field_type)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "string_value", setTarget), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameShortTest, set_static_field_by_name_long_invalid_args_object)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(nullptr, "short_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameShortTest, set_static_field_by_name_long_invalid_args_field)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, nullptr, setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameShortTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);
    ani_short single = 1U;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", static_cast<ani_short>(0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(0));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", static_cast<ani_short>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(NULL));

    ani_short max = std::numeric_limits<ani_short>::max();
    ani_short min = -std::numeric_limits<ani_short>::max();

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", static_cast<ani_short>(max)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(max));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", static_cast<ani_short>(min)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_short>(min));
}

TEST_F(ClassSetStaticFieldByNameShortTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    const ani_short setTarget2 = 3U;
    ani_short single = 1U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "short_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameShortTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_short_test.ShortStaticA", "short_value");
}

TEST_F(ClassSetStaticFieldByNameShortTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_short_test.ShortStaticFinal", "short_value");
}

TEST_F(ClassSetStaticFieldByNameShortTest, invalid_argument1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Short(nullptr, cls, "short_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameShortTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_short_test.ShortStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_short_test.ShortStatic"));
    const ani_short shortValue = 10;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_valuex", shortValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_short_test.ShortStatic"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Short(cls, "short_value", shortValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_short_test.ShortStatic"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
