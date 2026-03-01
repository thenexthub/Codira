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

class ClassSetStaticFieldLongTest : public AniTest {
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

TEST_F(ClassSetStaticFieldLongTest, set_long)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 24L);
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, 30L), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 30L);
}

TEST_F(ClassSetStaticFieldLongTest, set_long_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Long(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 24L);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Long(env_, cls, field, 30L), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Long(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 30L);
}

TEST_F(ClassSetStaticFieldLongTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, 30L), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldLongTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 24L);
    ASSERT_EQ(env_->Class_SetStaticField_Long(nullptr, field, 30L), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldLongTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, nullptr, 30L), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldLongTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long result = 0L;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Long(nullptr, cls, field, result), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldLongTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_long single = 0L;
    ani_long max = std::numeric_limits<ani_long>::max();
    ani_long min = -std::numeric_limits<ani_long>::max();
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, max), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);

    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, min), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Long(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
}

TEST_F(ClassSetStaticFieldLongTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLong", &cls), ANI_OK);
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

TEST_F(ClassSetStaticFieldLongTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_long_test.TestSetLongA", "long_value");
}

TEST_F(ClassSetStaticFieldLongTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_long_test.TestSetLongFinal", "long_value");
}

TEST_F(ClassSetStaticFieldLongTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_long_test.TestSetLongFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "long_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_long_test.TestSetLongFinal"));
    const ani_long longValue = 20L;
    ASSERT_EQ(env_->Class_SetStaticField_Long(cls, field, longValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_long_test.TestSetLongFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
