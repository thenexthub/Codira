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

class ClassSetStaticFieldByteTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_byte result = 0U;
        const ani_byte target = 126;
        ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_byte setTar = 127;
        ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassSetStaticFieldByteTest, set_byte)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0U;
    const ani_byte target = 126;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldByteTest, set_byte_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0U;
    const ani_byte target = 126;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Byte(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Byte(env_, cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Byte(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldByteTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTar), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByteTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0U;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
    const ani_byte target = 126;
    ASSERT_EQ(result, target);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->Class_SetStaticField_Byte(nullptr, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByteTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, nullptr, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByteTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const ani_byte setTar = 127;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Byte(nullptr, cls, field, setTar), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByteTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte single = 0U;
    ani_byte maxValue = std::numeric_limits<ani_byte>::max();
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, maxValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, maxValue);

    ani_byte minValue = std::numeric_limits<ani_byte>::min();
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, minValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minValue);
}

TEST_F(ClassSetStaticFieldByteTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_byte setTarget = 127;
    const ani_byte setTarget2 = 125;
    ani_byte single = 0U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByte", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTarget), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, setTarget);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTarget2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, setTarget2);
}

TEST_F(ClassSetStaticFieldByteTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_byte_test.TestSetByte", "byte_value");
}

TEST_F(ClassSetStaticFieldByteTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_byte_test.TestSetByteA", "byte_value");
}

TEST_F(ClassSetStaticFieldByteTest, combination_test4)
{
    CheckFieldValue("class_set_static_field_byte_test.TestSetByteFinal", "byte_value");
}

TEST_F(ClassSetStaticFieldByteTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_byte_test.TestSetByteFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_byte_test.TestSetByteFinal"));
    const ani_byte byteValue = 127;
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, byteValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_byte_test.TestSetByteFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
