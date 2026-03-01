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

class ClassSetStaticFieldByNameCharTest : public AniTest {
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

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_capi)
{
    ani_class cls {};
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Char(env_, cls, "char_value", target), ANI_OK);
    ani_char resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Char(env_, cls, "char_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, target);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char)
{
    ani_class cls {};
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", target), ANI_OK);
    ani_char resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, target);
}

TEST_F(ClassSetStaticFieldByNameCharTest, not_found)
{
    ani_class cls {};
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "charValue", target), ANI_NOT_FOUND);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "string_value", 'b'), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_args_object)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(nullptr, "char_value", 'b'), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_args_field)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, nullptr, 'b'), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameCharTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);
    ani_char single = ' ';

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>(NULL));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\n')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\n'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\r')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\r'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\t')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\t'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\b')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\b'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\a')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\a'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\v')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\v'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\f')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\f'));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", static_cast<ani_char>('\0')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_char>('\0'));

    ani_char maxValue = std::numeric_limits<ani_char>::max();
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", maxValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, maxValue);

    ani_char minValue = std::numeric_limits<ani_char>::min();
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", minValue), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, minValue);
}

TEST_F(ClassSetStaticFieldByNameCharTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    const ani_short setTarget2 = 3U;
    ani_char single = ' ';
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameCharTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_char_test.CharStaticA", "char_value");
}

TEST_F(ClassSetStaticFieldByNameCharTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_char_test.CharStaticFinal", "char_value");
}

TEST_F(ClassSetStaticFieldByNameCharTest, invalid_argument1)
{
    ani_class cls {};
    const ani_char setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Char(nullptr, cls, "char_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameCharTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_char_test.CharStatic", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_char_test.CharStatic"));
    const ani_char charValue = 'n';

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_valuex", charValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_char_test.CharStatic"));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", charValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_char_test.CharStatic"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
