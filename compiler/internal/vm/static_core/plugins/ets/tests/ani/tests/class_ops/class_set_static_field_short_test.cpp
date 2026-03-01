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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassSetStaticFieldShortTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_short result = 0U;
        const ani_short target = 3U;
        ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_short setTar = 30U;
        ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassSetStaticFieldShortTest, set_short)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, 7U), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 7U);
}

TEST_F(ClassSetStaticFieldShortTest, set_short_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Short(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Short(env_, cls, field, 7U), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Short(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 7U);
}

TEST_F(ClassSetStaticFieldShortTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, 7U), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldShortTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->Class_SetStaticField_Short(nullptr, field, 7U), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldShortTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, nullptr, 7U), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldShortTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Short(nullptr, cls, field, result), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldShortTest, special_values)
{
    ani_class cls {};
    ani_static_field field {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short single = 0;
    ani_short max = std::numeric_limits<ani_short>::max();
    ani_short min = std::numeric_limits<ani_short>::min();
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, max), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);

    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, min), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
}

TEST_F(ClassSetStaticFieldShortTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_short setTarget = 127U;
    const ani_short setTarget2 = 125U;
    ani_short single = 0U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShort", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, setTarget), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, setTarget);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, setTarget2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, setTarget2);
}

TEST_F(ClassSetStaticFieldShortTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_short_test.TestSetShort", "short_value");
}

TEST_F(ClassSetStaticFieldShortTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_short_test.TestSetShortA", "short_value");
}

TEST_F(ClassSetStaticFieldShortTest, combination_test4)
{
    CheckFieldValue("class_set_static_field_short_test.TestSetShortFinal", "short_value");
}

TEST_F(ClassSetStaticFieldShortTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_short_test.TestSetShortFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_short_test.TestSetShortFinal"));
    const ani_short shortValue = 127U;
    ASSERT_EQ(env_->Class_SetStaticField_Short(cls, field, shortValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_short_test.TestSetShortFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
