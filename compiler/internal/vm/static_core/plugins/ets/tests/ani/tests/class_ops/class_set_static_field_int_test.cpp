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

class ClassSetStaticFieldIntTest : public AniTest {};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define CHECK_FIELD_VALUE(cls, field, target, setTar)                           \
    do {                                                                        \
        ani_int result = 0U;                                                    \
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK); \
        ASSERT_EQ(result, target);                                              \
        ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar), ANI_OK);  \
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK); \
        ASSERT_EQ(result, setTar);                                              \
        ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, target), ANI_OK);  \
    } while (0)
// NOLINTEND(cppcoreguidelines-macro-usage)

TEST_F(ClassSetStaticFieldIntTest, set_int)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, 1024U), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 1024U);
}

TEST_F(ClassSetStaticFieldIntTest, set_int_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Int(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Int(env_, cls, field, 1024U), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Int(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 1024U);
}

TEST_F(ClassSetStaticFieldIntTest, set_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, 1024U), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldIntTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ASSERT_EQ(env_->Class_SetStaticField_Int(nullptr, field, 1024U), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldIntTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, nullptr, 1024U), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldIntTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Int(nullptr, cls, field, result), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldIntTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int single = 0;
    ani_int max = std::numeric_limits<ani_int>::max();
    ani_int min = -std::numeric_limits<ani_int>::max();

    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, max), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);

    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, min), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
}

TEST_F(ClassSetStaticFieldIntTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetInt", &cls), ANI_OK);
    ani_static_field field {};
    const ani_int setTar = 1024;
    const ani_int setTar2 = 10;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar2);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassSetStaticFieldIntTest, combination_test2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetIntA", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    CHECK_FIELD_VALUE(cls, field, 3U, 2U);
}

TEST_F(ClassSetStaticFieldIntTest, combination_test3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetIntFinal", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    CHECK_FIELD_VALUE(cls, field, 3U, 2U);
}

TEST_F(ClassSetStaticFieldIntTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.TestSetIntFinal", &cls), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_int_test.TestSetIntFinal"));
    const ani_int intValue = 2410;
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_int_test.TestSetIntFinal"));
}

TEST_F(ClassSetStaticFieldIntTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.Parent", &clsParent), ANI_OK);
    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_int_test.Child", &clsChild), ANI_OK);

    ani_static_field parentFieldInParent {};
    ASSERT_EQ(env_->Class_FindStaticField(clsParent, "parentField", &parentFieldInParent), ANI_OK);
    ani_static_field parentFieldInChild {};
    ASSERT_EQ(env_->Class_FindStaticField(clsChild, "parentField", &parentFieldInChild), ANI_OK);
    ani_static_field childFieldInParent {};
    ASSERT_EQ(env_->Class_FindStaticField(clsParent, "childField", &childFieldInParent), ANI_NOT_FOUND);
    ani_static_field childFieldInChild {};
    ASSERT_EQ(env_->Class_FindStaticField(clsChild, "childField", &childFieldInChild), ANI_OK);
    ani_static_field overridedFieldInParent {};
    ASSERT_EQ(env_->Class_FindStaticField(clsParent, "overridedField", &overridedFieldInParent), ANI_OK);
    ani_static_field overridedFieldInChild {};
    ASSERT_EQ(env_->Class_FindStaticField(clsChild, "overridedField", &overridedFieldInChild), ANI_OK);

    // |-------------------------------------------------------------------------------------------------------|
    // |  ani_class  |          ani_static_field          |  ani_status  |               action                |
    // |-------------|------------------------------------|--------------|-------------------------------------|
    // |   Parent    |   parentField from Parent class    |    ANI_OK    |    access to Parent.parentField     |
    // |   Parent    |   parentField from Child class     |    ANI_OK    |    access to Parent.parentField     |
    // |   Parent    |    childField from Child class     |      UB      |                 --                  |
    // |   Parent    |  overridedField from Child class   |      UB      |                 --                  |
    // |   Parent    |  overridedField from Parent class  |    ANI_OK    |   access to Parent.overridedField   |
    // |   Child     |    parentField from Parent class   |    ANI_OK    |    access to Parent.parentField     |
    // |   Child     |    parentField from Child class    |    ANI_OK    |    access to Parent.parentField     |
    // |   Child     |    childField from Child class     |    ANI_OK    |     access to Child.childField      |
    // |   Child     |  overridedField from Child class   |    ANI_OK    |   access to Child.overridedField    |
    // |   Child     |  overridedField from Parent class  |    ANI_OK    |   access to Parent.overridedField   |
    // |-------------|------------------------------------|--------------|-------------------------------------|

    CHECK_FIELD_VALUE(clsParent, parentFieldInParent, 0U, 1U);
    CHECK_FIELD_VALUE(clsParent, parentFieldInChild, 0U, 2U);
    CHECK_FIELD_VALUE(clsParent, overridedFieldInParent, 0U, 3U);

    CHECK_FIELD_VALUE(clsChild, parentFieldInParent, 0U, 4U);
    CHECK_FIELD_VALUE(clsChild, parentFieldInChild, 0U, 5U);
    CHECK_FIELD_VALUE(clsChild, childFieldInChild, 0U, 6U);
    CHECK_FIELD_VALUE(clsChild, overridedFieldInChild, 0U, 7U);
    CHECK_FIELD_VALUE(clsChild, overridedFieldInParent, 0U, 8U);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
