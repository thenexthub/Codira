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

class ClassGetStaticFieldByNameDoubleTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_double setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, fieldName, setTarget), ANI_OK);
        ani_double resultValue = 0.0;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double age = 0.0;
    const ani_double expectedAge = 20.0;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Double(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_double>(expectedAge));
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double age = 0.0;
    const ani_double expectedAge = 20.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_double>(expectedAge));
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);

    ani_double age = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double age = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double age = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double age = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Double(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    ani_double single = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "specia1", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "specia5", &single), ANI_INVALID_TYPE);
    ani_double max = std::numeric_limits<ani_double>::max();
    ani_double minpositive = std::numeric_limits<ani_double>::min();
    ani_double min = -std::numeric_limits<ani_double>::max();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "doubleMin", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(min));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "minpositive", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(minpositive));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "doubleMax", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(max));
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, combination_test1)
{
    ani_class cls {};
    const ani_double setTarget = 2U;
    const ani_double setTarget2 = 3U;
    ani_double single = 0.0;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "age", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "age", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_double_test.Woman", "age");
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_double_test.DoubleStaticA", "double_value");
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_double_test.DoubleStaticFinal", "double_value");
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_double_test.Woman", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_double_test.Woman"));
    ani_double doubleValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "agex", &doubleValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_double_test.Woman"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "age", &doubleValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_double_test.Woman"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
