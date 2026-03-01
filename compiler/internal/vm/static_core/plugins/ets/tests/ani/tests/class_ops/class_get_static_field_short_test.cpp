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

class ClassGetStaticFieldShortTest : public AniTest {
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

TEST_F(ClassGetStaticFieldShortTest, get_short)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0U;
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
}

TEST_F(ClassGetStaticFieldShortTest, get_short_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0U;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Short(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
}

TEST_F(ClassGetStaticFieldShortTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0U;
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldShortTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0U;
    ASSERT_EQ(env_->Class_GetStaticField_Short(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldShortTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_short result = 0U;
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldShortTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldShortTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_short result = 0U;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Short(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldShortTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
    ani_short single;
    const ani_short maxTarget = 32767;
    const ani_short minTarget = -32768;
    ani_static_field field {};

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special2", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special4", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "special5", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "shortMax", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, maxTarget);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "shortMin", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minTarget);
}

TEST_F(ClassGetStaticFieldShortTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_short setTarget = 127U;
    const ani_short setTarget2 = 125U;
    ani_short single;
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);
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

TEST_F(ClassGetStaticFieldShortTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_short_test.TestShort", "short_value");
}

TEST_F(ClassGetStaticFieldShortTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_short_test.TestShortA", "short_value");
}

TEST_F(ClassGetStaticFieldShortTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_short_test.TestShortFinal", "short_value");
}

TEST_F(ClassGetStaticFieldShortTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_short_test.TestShort", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "short_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_short_test.TestShort"));
    ani_short shortValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Short(cls, field, &shortValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_short_test.TestShort"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
