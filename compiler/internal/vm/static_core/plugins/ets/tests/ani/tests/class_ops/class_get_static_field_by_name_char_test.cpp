/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific languname governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldByNameCharTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_char setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, fieldName, setTarget), ANI_OK);
        ani_char resultValue = ' ';
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Char(env_, cls, "name", &name), ANI_OK);
    ASSERT_EQ(name, static_cast<ani_char>('b'));
}

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", &name), ANI_OK);
    ASSERT_EQ(name, static_cast<ani_char>('b'));
}

TEST_F(ClassGetStaticFieldByNameCharTest, not_found)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "nameChar", &name), ANI_NOT_FOUND);
}

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);

    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "age", &name), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(nullptr, "name", &name), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, nullptr, &name), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char name = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "", &name), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "\n", &name), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Char(nullptr, cls, "name", &name), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    ani_char single = 'c';
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia2", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia5", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\n'));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia6", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\r'));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia7", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\t'));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia8", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\b'));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "specia12", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\0'));

    ani_char maxValue = std::numeric_limits<ani_char>::max();
    ani_char minValue = std::numeric_limits<ani_char>::min();
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "charMin", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>(minValue));
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "charMax", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>(maxValue));
}

TEST_F(ClassGetStaticFieldByNameCharTest, combination_test1)
{
    ani_class cls {};
    const ani_char setTarget = 2U;
    const ani_char setTarget2 = 3U;
    ani_char single = 'c';
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.GetCharStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "name", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "name", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassGetStaticFieldByNameCharTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_char_test.GetCharStatic", "name");
}

TEST_F(ClassGetStaticFieldByNameCharTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_char_test.CharStaticA", "char_value");
}

TEST_F(ClassGetStaticFieldByNameCharTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_char_test.CharStaticFinal", "char_value");
}

TEST_F(ClassGetStaticFieldByNameCharTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_char_test.CharStaticFinal", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_char_test.CharStaticFinal"));
    ani_char charValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_valuex", &charValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_char_test.CharStaticFinal"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &charValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_char_test.CharStaticFinal"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
