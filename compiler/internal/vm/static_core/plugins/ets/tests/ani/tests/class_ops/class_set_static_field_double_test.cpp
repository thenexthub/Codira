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

class ClassSetStaticFieldDoubleTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_double result = 0U;
        const ani_double target = 18.0;
        ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_double setTar = 20.0;
        ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassSetStaticFieldDoubleTest, set_double)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_double result = 0.0;
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
    const ani_double target = 18.0;
    ASSERT_EQ(result, target);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldDoubleTest, set_double_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_double result = 0.0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Double(env_, cls, field, &result), ANI_OK);
    const ani_double target = 18.0;
    ASSERT_EQ(result, target);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Double(env_, cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Double(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldDoubleTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, setTar), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldDoubleTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_double result = 0.0;
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
    const ani_double target = 18.0;
    ASSERT_EQ(result, target);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->Class_SetStaticField_Double(nullptr, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldDoubleTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, nullptr, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldDoubleTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_double setTar = 28.0;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Double(nullptr, cls, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldDoubleTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_double single = 0.0;
    ani_double max = std::numeric_limits<ani_double>::max();
    ani_double minpositive = std::numeric_limits<ani_double>::min();
    ani_double min = -std::numeric_limits<ani_double>::max();

    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, max), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);

    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, minpositive), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minpositive);

    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, min), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
}

TEST_F(ClassSetStaticFieldDoubleTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDouble", &cls), ANI_OK);
    ani_static_field field {};
    const ani_double setTar = 28.0;
    const ani_double setTar2 = 18.0;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_double result = 0.0;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, setTar2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar2);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Double(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldDoubleTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_double_test.TestSetDoubleA", "double_value");
}

TEST_F(ClassSetStaticFieldDoubleTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_double_test.TestSetDoubleFinal", "double_value");
}

TEST_F(ClassSetStaticFieldDoubleTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_double_test.TestSetDoubleFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "double_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_double_test.TestSetDoubleFinal"));
    const ani_double doubleValue = 16.05;
    ASSERT_EQ(env_->Class_SetStaticField_Double(cls, field, doubleValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_double_test.TestSetDoubleFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
