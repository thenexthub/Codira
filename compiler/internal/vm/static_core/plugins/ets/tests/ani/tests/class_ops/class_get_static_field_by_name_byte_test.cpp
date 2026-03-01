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

class ClassGetStaticFieldByNameByteTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_byte setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, fieldName, setTarget), ANI_OK);
        ani_byte resultValue = 1U;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Byte(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_byte>(0));
}

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, static_cast<ani_byte>(0));
}

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);

    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "\n", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Byte(nullptr, cls, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    ani_byte single = 1U;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia2", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia5", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia6", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia7", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia8", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia9", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia10", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia11", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "specia12", &single), ANI_INVALID_TYPE);

    const ani_byte maxValue = 127;
    const ani_byte minValue = -128;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byteMin", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>(minValue));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byteMax", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>(maxValue));
}

TEST_F(ClassGetStaticFieldByNameByteTest, combination_test1)
{
    ani_class cls {};
    const ani_byte setTarget = 2U;
    const ani_byte setTarget2 = 3U;
    ani_byte single = 1U;
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.GetbyteStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "age", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "age", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "age", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "age", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameByteTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_byte_test.GetbyteStatic", "age");
}

TEST_F(ClassGetStaticFieldByNameByteTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_byte_test.ByteStaticA", "byte_value");
}

TEST_F(ClassGetStaticFieldByNameByteTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_byte_test.ByteStaticFinal", "byte_value");
}

TEST_F(ClassGetStaticFieldByNameByteTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_byte_test.ByteStaticA", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_byte_test.ByteStaticA"));
    ani_byte byteValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_valuex", &byteValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_byte_test.ByteStaticA"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &byteValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_byte_test.ByteStaticA"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
