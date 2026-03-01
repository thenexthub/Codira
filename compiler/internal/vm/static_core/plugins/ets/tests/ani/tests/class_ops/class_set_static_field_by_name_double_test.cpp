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

class ClassSetStaticFieldByNameDoubleTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double_capi)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Double(env_, cls, "double_value", age), ANI_OK);
    ani_double resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Double(env_, cls, "double_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, age);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", age), ANI_OK);
    ani_double resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, age);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, invalid_argument1)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "string_value", age), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, invalid_argument2)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(nullptr, "double_value", age), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, invalid_argument3)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, nullptr, age), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, invalid_argument4)
{
    ani_class cls {};
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "", age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "\n", age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Double(nullptr, cls, "double_value", age), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", static_cast<ani_double>(0)), ANI_OK);
    ani_double single = 0.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(0));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", static_cast<ani_double>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(NULL));

    ani_double max = std::numeric_limits<ani_double>::max();
    ani_double minpositive = std::numeric_limits<ani_double>::min();
    ani_double min = -std::numeric_limits<ani_double>::max();

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", static_cast<ani_double>(max)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(max));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", static_cast<ani_double>(minpositive)),
              ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(minpositive));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", static_cast<ani_double>(min)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_double>(min));
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    const ani_short setTarget2 = 3U;
    ani_double single = 0.0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_double_test.DoubleStaticA", "double_value");
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_double_test.DoubleStaticFinal", "double_value");
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_double_test.DoubleStaticA", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_double_test.DoubleStaticA"));
    const ani_double doubleValue = 2.2;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_valuex", doubleValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_double_test.DoubleStaticA"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", doubleValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_double_test.DoubleStaticA"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
