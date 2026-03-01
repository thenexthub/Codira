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

class ClassSetStaticFieldByNameLongTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameLongTest, set_static_field_by_name_long_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Long(env_, cls, "long_value", 8L), ANI_OK);
    ani_long resultValue = 0L;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Long(env_, cls, "long_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 8L);
}

TEST_F(ClassSetStaticFieldByNameLongTest, set_static_field_by_name_long)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", 8L), ANI_OK);
    ani_long resultValue = 0L;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 8L);
}

TEST_F(ClassSetStaticFieldByNameLongTest, set_static_field_by_name_long_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "string_value", 8L), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameLongTest, set_static_field_by_name_long_invalid_args_object)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(nullptr, "long_value", 8L), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameLongTest, set_static_field_by_name_long_invalid_args_field)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, nullptr, 8L), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameLongTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);
    ani_long single = 0L;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", static_cast<ani_long>(0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(0));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", static_cast<ani_long>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(NULL));

    ani_long max = std::numeric_limits<ani_long>::max();
    ani_long min = -std::numeric_limits<ani_long>::max();

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", static_cast<ani_long>(max)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(max));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", static_cast<ani_long>(min)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_long>(min));
}

TEST_F(ClassSetStaticFieldByNameLongTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2L;
    const ani_short setTarget2 = 3L;
    ani_long single = 0L;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Long(cls, "long_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameLongTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_long_test.PackageStaticA", "long_value");
}

TEST_F(ClassSetStaticFieldByNameLongTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_long_test.PackageStaticFinal", "long_value");
}

TEST_F(ClassSetStaticFieldByNameLongTest, invalid_argument1)
{
    ani_class cls {};
    const ani_long setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Long(nullptr, cls, "long_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameLongTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_long_test.PackageStaticFinal", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_long_test.PackageStaticFinal"));
    const ani_long longValue = 10;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_valuex", longValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_long_test.PackageStaticFinal"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Long(cls, "long_value", longValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_long_test.PackageStaticFinal"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
