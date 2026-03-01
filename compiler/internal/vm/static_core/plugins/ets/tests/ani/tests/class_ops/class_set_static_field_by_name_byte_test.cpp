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

class ClassSetStaticFieldByNameByteTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameByteTest, set_static_field_by_name_byte_capi)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Byte(env_, cls, "byte_value", setTarget), ANI_OK);
    ani_byte resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Byte(env_, cls, "byte_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameByteTest, set_static_field_by_name_byte)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", setTarget), ANI_OK);
    ani_byte resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameByteTest, set_static_field_by_name_byte_invalid_field_type)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "string_value", setTarget), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameByteTest, set_static_field_by_name_byte_invalid_args_object)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(nullptr, "byte_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameByteTest, set_static_field_by_name_byte_invalid_args_field)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, nullptr, setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameByteTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);
    ani_byte single = 1U;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>(NULL));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\n')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\n'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\r')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\r'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\t')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\t'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\b')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\b'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\a')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\a'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\v')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\v'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\f')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\f'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", static_cast<ani_byte>('\0')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_byte>('\0'));

    ani_byte maxValue = std::numeric_limits<ani_byte>::max();
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", maxValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, maxValue);

    ani_byte minValue = std::numeric_limits<ani_byte>::min();
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", minValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, minValue);
}

TEST_F(ClassSetStaticFieldByNameByteTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 3U;
    const ani_short setTarget2 = 2U;
    ani_byte single = 1U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", setTarget), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", setTarget2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "byte_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget2);
}

TEST_F(ClassSetStaticFieldByNameByteTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_byte_test.ByteStaticA", "byte_value");
}

TEST_F(ClassSetStaticFieldByNameByteTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_byte_test.ByteStaticFinal", "byte_value");
}

TEST_F(ClassSetStaticFieldByNameByteTest, invalid_argument)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Byte(nullptr, cls, "byte_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameByteTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_byte_test.ByteStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_byte_test.ByteStatic"));
    const ani_short byteValue = 1U;

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_valuex", byteValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_byte_test.ByteStatic"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Byte(cls, "byte_value", byteValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_byte_test.ByteStatic"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
