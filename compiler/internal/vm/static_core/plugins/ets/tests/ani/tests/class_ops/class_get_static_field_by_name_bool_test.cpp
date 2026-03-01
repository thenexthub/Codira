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
 * See the License for the specific langusingle governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldByNameBoolTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, fieldName, ANI_TRUE), ANI_OK);
        ani_boolean resultValue = ANI_FALSE;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, ANI_TRUE);
    }
};

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool_c_api_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Boolean(env_, cls, "single", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "single", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);

    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "name", &single), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(nullptr, "name", &single), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, nullptr, &single), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "", &single), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "\n", &single), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Boolean(nullptr, cls, "single", &single), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia1", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia3", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia4", &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia5", &single), ANI_INVALID_TYPE);

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia6", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia7", &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia8", &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "specia9", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.GetBoolStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "single", ANI_TRUE), ANI_OK);
        ani_boolean single = ANI_FALSE;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "single", &single), ANI_OK);
        ASSERT_EQ(single, ANI_TRUE);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "single", ANI_FALSE), ANI_OK);
    ani_boolean single = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "single", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, combination_test2)
{
    CheckFieldValue("class_get_static_field_by_name_bool_test.GetBoolStatic", "single");
}

TEST_F(ClassGetStaticFieldByNameBoolTest, combination_test3)
{
    CheckFieldValue("class_get_static_field_by_name_bool_test.BoolStaticA", "bool_value");
}

TEST_F(ClassGetStaticFieldByNameBoolTest, combination_test4)
{
    CheckFieldValue("class_get_static_field_by_name_bool_test.BoolStaticFinal", "bool_value");
}

TEST_F(ClassGetStaticFieldByNameBoolTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_get_static_field_by_name_bool_test.BoolStaticA", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_bool_test.BoolStaticA"));
    ani_boolean boolValue {};

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_valuex", &boolValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_get_static_field_by_name_bool_test.BoolStaticA"));

    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &boolValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_get_static_field_by_name_bool_test.BoolStaticA"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
