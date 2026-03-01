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

class ClassGetStaticFieldByteTest : public AniTest {
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

TEST_F(ClassGetStaticFieldByteTest, get_byte)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    const ani_byte target = 126;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldByteTest, get_byte_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    const ani_byte target = 126;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Byte(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldByteTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Byte(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);

    ani_static_field field {};
    ani_byte single = 0;
    const int32_t fieldNum = 11;
    for (int32_t i = 1; i <= fieldNum; ++i) {
        std::string fieldName = "special" + std::to_string(i);
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName.c_str(), &field), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_INVALID_TYPE);
    }

    const ani_byte maxTarget = 127;
    const ani_byte minTarget = -128;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byteMax", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, maxTarget);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "byteMin", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, minTarget);
}

TEST_F(ClassGetStaticFieldByteTest, combination_test1)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_byte setTarget = 127;
    ani_byte single = 0U;
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTarget), ANI_OK);

    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByteTest, combination_test2)
{
    ani_class cls {};
    ani_static_field field {};
    const ani_byte setTarget = 127;
    const ani_byte setTarget2 = 125;
    ani_byte single = 0U;
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByte", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum - 1; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTarget), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
        ASSERT_EQ(single, setTarget);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Byte(cls, field, setTarget2), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, setTarget2);
}

TEST_F(ClassGetStaticFieldByteTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_byte_test.TestByteA", "byte_value");
}

TEST_F(ClassGetStaticFieldByteTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_byte_test.TestByteFinal", "byte_value");
}

TEST_F(ClassGetStaticFieldByteTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_byte_test.TestByteFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_byte_test.TestByteFinal"));
    ani_byte byteValue {};
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &byteValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_byte_test.TestByteFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
