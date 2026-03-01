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

class ClassGetStaticFieldLongTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_long result = 0U;
        const ani_long target = 24L;
        ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_long setTar = 30L;
        ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassGetStaticFieldLongTest, get_long)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 24L);
}

TEST_F(ClassGetStaticFieldLongTest, get_long_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Long(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 24L);
}

TEST_F(ClassGetStaticFieldLongTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldLongTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldLongTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldLongTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldLongTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Long(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldLongTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    ani_long single = 0L;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia1", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia4", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia5", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_INVALID_TYPE);
    ani_long max = std::numeric_limits<ani_long>::max();
    ani_long min = std::numeric_limits<ani_long>::min();
    ASSERT_EQ(env_->Class_FindStaticField(cls, "min", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "max", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);
}

TEST_F(ClassGetStaticFieldLongTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);
    ani_static_field field {};
    const ani_long setTar = 30L;
    const ani_long setTar2 = 10L;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, setTar2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar2);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassGetStaticFieldLongTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_long_test.TestLong", "long_value");
}

TEST_F(ClassGetStaticFieldLongTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_long_test.TestLongA", "long_value");
}

TEST_F(ClassGetStaticFieldLongTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_long_test.TestLongFinal", "long_value");
}

TEST_F(ClassGetStaticFieldLongTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_long_test.TestLong", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_long_test.TestLong"));
    ani_long longValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &longValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_long_test.TestLong"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
